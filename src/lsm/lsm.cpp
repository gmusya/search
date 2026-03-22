#include "src/lsm/lsm.h"

namespace search {

void Lsm::CollapseEntries(std::vector<std::pair<InternalKey, Value>>& entries) const {
  if (!merge_operator_ || entries.empty()) {
    return;
  }

  std::vector<std::pair<InternalKey, Value>> collapsed;
  collapsed.reserve(entries.size());

  collapsed.push_back(std::move(entries[0]));

  for (size_t i = 1; i < entries.size(); ++i) {
    if (collapsed.back().first.key == entries[i].first.key) {
      collapsed.back().second = merge_operator_->Merge(collapsed.back().second, entries[i].second);
      collapsed.back().first.is_deleted = false;
    } else {
      collapsed.push_back(std::move(entries[i]));
    }
  }

  entries = std::move(collapsed);
}

std::optional<Value> Lsm::Get(const Key& key) {
  std::optional<Value> accumulated;

  Value memtable_result;
  MemTable::Status status = memtable_->Get(key, &memtable_result, merge_operator_.get());

  if (status == MemTable::Status::kDeleted && !merge_operator_) {
    return std::nullopt;
  }
  if (status == MemTable::Status::kFound) {
    accumulated = std::move(memtable_result);
    if (!merge_operator_) {
      return accumulated;
    }
  }

  // Check SSTables
  for (const auto& level : sstables_) {
    for (const SSTableInfo& info : level) {
      if (info.min <= key && key <= info.max) {
        std::shared_ptr<IFile> bloom_filter_file = filesystem_->Open(info.bloom_filter_path);
        BloomFilterReader bloom_filter(bloom_filter_file);

        if (!bloom_filter.Contains(key)) {
          continue;
        }

        std::shared_ptr<IFile> file = filesystem_->Open(info.sstable_path);
        SSTableReader reader(file);

        Value result;
        SSTableReader::Status s = reader.Get(key, &result);

        if (s == SSTableReader::Status::kDeleted && !merge_operator_) {
          return std::nullopt;
        }

        if (s == SSTableReader::Status::kFound) {
          if (accumulated && merge_operator_) {
            accumulated = merge_operator_->Merge(*accumulated, result);
          } else {
            accumulated = std::move(result);
          }
          if (!merge_operator_) {
            return accumulated;
          }
        }
      }
    }
  }

  return accumulated;
}

void Lsm::Insert(const Key& key, const Value& value) {
  memtable_->Add(++sequence_number_, key, value);
  if (memtable_->Bytes() >= params_.memtable_bytes_limit) {
    FlushMemTable();
  }
}

void Lsm::Delete(const Key& key) {
  memtable_->Delete(++sequence_number_, key);
  if (memtable_->Bytes() >= params_.memtable_bytes_limit) {
    FlushMemTable();
  }
}

std::vector<std::pair<Key, Value>> Lsm::ReadRange(std::optional<Key> min, std::optional<Key> max) {
  std::vector<std::pair<InternalKey, Value>> memtable_range = memtable_->ReadRange(min, max);

  for (const auto& level : sstables_) {
    for (const SSTableInfo& info : level) {
      if ((!min || info.max >= *min) && (!max || info.min <= *max)) {
        std::shared_ptr<IFile> file = filesystem_->Open(info.sstable_path);
        SSTableReader reader(file);

        auto sstable_range = reader.ReadRange(min, max);
        std::vector<std::pair<InternalKey, Value>> result;

        std::merge(memtable_range.begin(), memtable_range.end(), sstable_range.begin(), sstable_range.end(),
                   std::back_inserter(result));

        std::swap(memtable_range, result);
      }
    }
  }

  if (merge_operator_) {
    CollapseEntries(memtable_range);

    std::vector<std::pair<Key, Value>> cleared_result;
    cleared_result.reserve(memtable_range.size());
    for (auto& [k, v] : memtable_range) {
      cleared_result.emplace_back(std::move(k.key), std::move(v));
    }
    return cleared_result;
  }

  memtable_range.erase(std::unique(memtable_range.begin(), memtable_range.end(),
                                   [&](const auto& lhs, const auto& rhs) { return lhs.first.key == rhs.first.key; }),
                       memtable_range.end());

  std::vector<std::pair<Key, Value>> cleared_result;
  cleared_result.reserve(memtable_range.size());

  for (auto& [k, v] : memtable_range) {
    if (k.is_deleted) {
      continue;
    }
    cleared_result.emplace_back(std::move(k.key), std::move(v));
  }

  return cleared_result;
}

void Lsm::FlushMemTable() {
  std::vector<std::pair<InternalKey, Value>> values = memtable_->Values();
  ASSERT(!values.empty());

  CollapseEntries(values);

  std::string new_path = filesystem_->Create();
  std::shared_ptr<IFile> file = filesystem_->Open(new_path);

  SSTableWriter writer(file);

  std::string bloom_filter_path = filesystem_->Create();
  std::shared_ptr<IFile> bloom_filter_file = filesystem_->Open(bloom_filter_path);

  BloomFilterWriter bloom_filter_writer(
      bloom_filter_file,
      BloomFilterParameters{.bytes = std::max(1u, static_cast<uint32_t>(values.size() / 8)), .functions = 2});
  for (const auto& val : values) {
    bloom_filter_writer.Add(val.first.key);
  }

  std::move(bloom_filter_writer).Finish();

  SSTableInfo new_sstable_info{.min = values[0].first.key,
                               .max = values.back().first.key,
                               .sstable_path = new_path,
                               .bloom_filter_path = bloom_filter_path};

  std::move(writer).Write(values);

  if (sstables_.empty()) {
    sstables_.emplace_back();
  }

  sstables_[0].emplace_back(new_sstable_info);
  size_t current_level = 0;
  while (sstables_[current_level].size() == 2) {
    SSTableReader reader0(filesystem_->Open(sstables_[current_level][0].sstable_path));
    SSTableReader reader1(filesystem_->Open(sstables_[current_level][1].sstable_path));

    auto values0 = reader0.ReadRange(std::nullopt, std::nullopt);
    auto values1 = reader1.ReadRange(std::nullopt, std::nullopt);

    std::vector<std::pair<InternalKey, Value>> result;
    result.reserve(values0.size() + values1.size());

    std::merge(values0.begin(), values0.end(), values1.begin(), values1.end(), std::back_inserter(result));

    CollapseEntries(result);

    std::string new_path = filesystem_->Create();
    std::string bloom_filter_path = filesystem_->Create();

    SSTableInfo new_info{.min = result[0].first.key,
                         .max = result.back().first.key,
                         .sstable_path = new_path,
                         .bloom_filter_path = bloom_filter_path};

    std::shared_ptr<IFile> bloom_filter_file = filesystem_->Open(bloom_filter_path);

    BloomFilterWriter bloom_filter_writer(
        bloom_filter_file,
        BloomFilterParameters{.bytes = std::max(1u, static_cast<uint32_t>(result.size() / 8)), .functions = 2});
    for (const auto& val : result) {
      bloom_filter_writer.Add(val.first.key);
    }

    std::move(bloom_filter_writer).Finish();

    std::shared_ptr<IFile> new_file = filesystem_->Open(new_path);
    SSTableWriter(new_file).Write(result);

    filesystem_->Remove(sstables_[current_level][0].sstable_path);
    filesystem_->Remove(sstables_[current_level][1].sstable_path);
    filesystem_->Remove(sstables_[current_level][0].bloom_filter_path);
    filesystem_->Remove(sstables_[current_level][1].bloom_filter_path);

    if (current_level + 1 == sstables_.size()) {
      sstables_.emplace_back();
    }

    sstables_[current_level] = {};
    sstables_[current_level + 1].emplace_back(new_info);

    ++current_level;
  }

  memtable_.emplace();
}

}  // namespace search
