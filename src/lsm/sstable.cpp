#include "src/lsm/sstable.h"

#include <limits>

namespace search {

namespace serde {
template <typename T>
void Write(uint8_t*& bytes, const T& data) {
  std::memcpy(bytes, &data, sizeof(data));
  bytes += sizeof(data);
}

template <>
void Write(uint8_t*& bytes, const std::vector<uint8_t>& data) {
  Write<uint32_t>(bytes, data.size());
  if (!data.empty()) {
    std::memcpy(bytes, data.data(), data.size());
    bytes += data.size();
  }
}
}  // namespace serde

void SSTableReader::Load() {
  file_size_ = file_->Size();
  if (file_size_ == 0) {
    entries_count_ = 0;
    return;
  }

  ASSERT(file_size_ >= sizeof(uint32_t));

  file_->Read(0, sizeof(uint32_t), &entries_count_);

  const uint64_t offsets_bytes = static_cast<uint64_t>(entries_count_) * sizeof(uint32_t);

  [[maybe_unused]] const uint64_t offsets_region_size = sizeof(uint32_t) + offsets_bytes;
  ASSERT(offsets_region_size <= file_size_);
}

uint32_t SSTableReader::OffsetAt(uint32_t index) const {
  ASSERT(index < entries_count_);

  const uint64_t offset_pos = sizeof(uint32_t) + static_cast<uint64_t>(index) * sizeof(uint32_t);
  uint32_t entry_offset;
  file_->Read(offset_pos, sizeof(uint32_t), &entry_offset);
  ASSERT(entry_offset < file_size_);
  return entry_offset;
}

InternalKey SSTableReader::ReadKey(uint32_t offset) const {
  if (offset >= file_size_) {
    throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": entry offset out of range");
  }

  InternalKey key;
  file_->Read(offset, sizeof(uint64_t), &key.sequence_number);
  file_->Read(offset + sizeof(uint64_t), sizeof(uint8_t), &key.is_deleted);

  uint32_t key_size;
  file_->Read(offset + sizeof(uint64_t) + sizeof(uint8_t), sizeof(uint32_t), &key_size);

  const uint64_t user_key_offset = offset + sizeof(uint64_t) + sizeof(uint8_t) + sizeof(uint32_t);
  ASSERT(user_key_offset + key_size <= file_size_);

  key.key.resize(key_size);

  if (key_size > 0) {
    file_->Read(user_key_offset, key_size, key.key.data());
  }

  return key;
}

std::pair<InternalKey, Value> SSTableReader::ReadEntry(uint32_t offset) const {
  InternalKey key = ReadKey(offset);

  const uint32_t new_offset = offset + sizeof(uint64_t) + sizeof(uint8_t) + sizeof(uint32_t) + key.key.size();

  uint32_t value_size;
  file_->Read(new_offset, sizeof(uint32_t), &value_size);

  Value value(value_size);
  if (value_size > 0) {
    file_->Read(new_offset + sizeof(uint32_t), value_size, value.data());
  }

  return {std::move(key), std::move(value)};
}

uint32_t SSTableReader::SearchForKey(Key key) const {
  InternalKey probe;
  probe.key = key;
  probe.sequence_number = std::numeric_limits<uint64_t>::max();
  probe.is_deleted = false;

  size_t left = 0;
  size_t right = entries_count_;
  while (left < right) {
    size_t mid = left + (right - left) / 2;
    InternalKey mid_key = ReadKey(OffsetAt(mid));
    if (mid_key < probe) {
      left = mid + 1;
    } else {
      right = mid;
    }
  }

  return left;
}

std::vector<std::pair<InternalKey, Value>> SSTableReader::ReadRange(std::optional<Key> min,
                                                                    std::optional<Key> max) const {
  uint32_t left = min ? SearchForKey(*min) : 0;
  uint32_t right = max ? SearchForKey(*max) : entries_count_;
  if (left > 0) {
    --left;
  }
  if (right < entries_count_) {
    ++right;
  }

  std::vector<std::pair<InternalKey, Value>> result;
  for (uint32_t index = left; index < right; ++index) {
    auto [key, value] = ReadEntry(OffsetAt(index));
    if ((!min || key.key >= *min) && (!max || key.key <= *max)) {
      result.emplace_back(std::move(key), std::move(value));
    }
  }

  return result;
}

SSTableReader::Status SSTableReader::Get(const Key& search_key, Value* result) const {
  uint32_t position = SearchForKey(search_key);

  if (position >= entries_count_) {
    return Status::kNotFound;
  }

  auto [key, value] = ReadEntry(OffsetAt(position));
  if (key.key != search_key) {
    return Status::kNotFound;
  }

  if (key.is_deleted) {
    *result = Value{};
    return Status::kDeleted;
  }

  *result = std::move(value);
  return Status::kFound;
}

void SSTableWriter::Write(const std::vector<std::pair<InternalKey, Value>>& entries) && {
  uint32_t bytes_for_size = sizeof(uint32_t);
  uint32_t bytes_for_offsets = sizeof(uint32_t) * entries.size();
  uint32_t bytes_for_entries = 0;
  for (const auto& [k, v] : entries) {
    bytes_for_entries +=
        k.key.size() + v.size() + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint8_t);
  }

  std::vector<uint8_t> bytes(bytes_for_size + bytes_for_offsets + bytes_for_entries);

  uint8_t* offsets_data = bytes.data();
  serde::Write<uint32_t>(offsets_data, entries.size());

  uint8_t* entries_data = bytes.data() + bytes_for_size + bytes_for_offsets;

  for (uint32_t i = 0; i < entries.size(); ++i) {
    const auto& [k, v] = entries[i];

    serde::Write<uint32_t>(offsets_data, entries_data - bytes.data());
    serde::Write<uint64_t>(entries_data, k.sequence_number);
    serde::Write<uint8_t>(entries_data, static_cast<uint8_t>(k.is_deleted));
    serde::Write<std::vector<uint8_t>>(entries_data, k.key);
    serde::Write<std::vector<uint8_t>>(entries_data, v);
  }

  if (bytes.data() + bytes.size() != entries_data) {
    throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": internal error");
  }

  file_->Resize(bytes.size());
  file_->Write(0, bytes.data(), bytes.size());
}

}  // namespace search
