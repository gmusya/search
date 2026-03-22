#include "src/lsm/memtable.h"

#include <limits>

namespace search {

void MemTable::Add(SequenceNumber sequence_number, const Key& key, const Value& value) {
  InternalKey ikey{key, sequence_number, false};
  approx_bytes_ += static_cast<uint64_t>(sizeof(InternalKey) + key.size() + sizeof(Value) + value.size());
  entries_.emplace(std::move(ikey), value);
}

void MemTable::Delete(SequenceNumber sequence_number, const Key& key) {
  InternalKey ikey{key, sequence_number, true};
  Value empty;
  approx_bytes_ += static_cast<uint64_t>(sizeof(InternalKey) + key.size() + sizeof(Value));
  entries_.emplace(std::move(ikey), std::move(empty));
}

MemTable::Status MemTable::Get(const Key& key, Value* result) const {
  InternalKey seek{key, std::numeric_limits<uint64_t>::max(), false};
  auto it = entries_.lower_bound(seek);
  if (it == entries_.end() || it->first.key != seek.key) {
    return Status::kNotFound;
  }
  if (it->first.is_deleted) {
    *result = {};
    return Status::kDeleted;
  }
  *result = it->second;
  return Status::kFound;
}

std::vector<std::pair<InternalKey, Value>> MemTable::ReadRange(std::optional<Key> min, std::optional<Key> max) {
  auto begin =
      min ? entries_.lower_bound(InternalKey{*min, std::numeric_limits<uint64_t>::max(), false}) : entries_.begin();
  auto end = max ? entries_.upper_bound(InternalKey{*max, 0, false}) : entries_.end();

  std::vector<std::pair<InternalKey, Value>> result;
  for (auto it = begin; it != end; ++it) {
    result.emplace_back(it->first, it->second);
  }

  return result;
}

std::vector<std::pair<InternalKey, Value>> MemTable::Values() const {
  std::vector<std::pair<InternalKey, Value>> result;
  result.reserve(entries_.size());

  for (const auto& [k, v] : entries_) {
    result.emplace_back(k, v);
  }

  return result;
}

}  // namespace search
