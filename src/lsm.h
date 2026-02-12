#pragma once

#include <algorithm>
#include <iostream>
#include <iterator>
#include <optional>
#include <vector>

#include "src/bloom_filter.h"
#include "src/file.h"
#include "src/filesystem.h"
#include "src/memtable.h"
#include "src/sstable.h"
#include "src/types.h"

namespace search {

class Lsm {
 public:
  struct Parameters {
    uint32_t memtable_bytes_limit = 65536;

    static Parameters Default() { return Parameters{}; }
  };

  explicit Lsm(std::shared_ptr<IFileSystem> filesystem, Parameters params = Parameters::Default())
      : params_(params), filesystem_(filesystem) {
    memtable_.emplace();
  }

  std::optional<Value> Get(const Key& key) {
    Value result;
    MemTable::Status status = memtable_->Get(key, &result);

    if (status == MemTable::Status::kDeleted) {
      return std::nullopt;
    }
    if (status == MemTable::Status::kFound) {
      return result;
    }

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

          SSTableReader::Status s = reader.Get(key, &result);
          if (s == SSTableReader::Status::kDeleted) {
            return std::nullopt;
          }

          if (s == SSTableReader::Status::kFound) {
            return result;
          }
        }
      }
    }

    return std::nullopt;
  }

  void Insert(const Key& key, const Value& value) {
    memtable_->Add(++sequence_number_, key, value);
    if (memtable_->Bytes() >= params_.memtable_bytes_limit) {
      FlushMemTable();
    }
  }

  void Delete(const Key& key) {
    memtable_->Delete(++sequence_number_, key);
    if (memtable_->Bytes() >= params_.memtable_bytes_limit) {
      FlushMemTable();
    }
  }

  std::vector<std::pair<Key, Value>> ReadRange(std::optional<Key> min, std::optional<Key> max) {
    std::vector<std::pair<InternalKey, Value>> memtable_range = memtable_->ReadRange(min, max);

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

 private:
  void FlushMemTable() {
    std::vector<std::pair<InternalKey, Value>> values = memtable_->Values();
    ASSERT(!values.empty());

    std::string new_path = filesystem_->Create();
    std::shared_ptr<IFile> file = filesystem_->Open(new_path);

    SSTableWriter writer(file);

    std::string bloom_filter_path = filesystem_->Create();
    std::shared_ptr<IFile> bloom_filter_file = filesystem_->Open(bloom_filter_path);

    BloomFilterWriter bloom_filter_writer(
        bloom_filter_file, BloomFilterParameters{.bytes = static_cast<uint32_t>(values.size() / 8), .functions = 2});
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

      std::string new_path = filesystem_->Create();
      std::string bloom_filter_path = filesystem_->Create();

      SSTableInfo new_info{.min = result[0].first.key,
                           .max = result.back().first.key,
                           .sstable_path = new_path,
                           .bloom_filter_path = bloom_filter_path};

      std::shared_ptr<IFile> bloom_filter_file = filesystem_->Open(bloom_filter_path);

      BloomFilterWriter bloom_filter_writer(
          bloom_filter_file, BloomFilterParameters{.bytes = static_cast<uint32_t>(result.size() / 8), .functions = 2});
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

  struct SSTableInfo {
    Key min;
    Key max;
    std::string sstable_path;
    std::string bloom_filter_path;
  };

  const Parameters params_;

  SequenceNumber sequence_number_ = 0;

  std::shared_ptr<IFileSystem> filesystem_;

  std::vector<std::vector<SSTableInfo>> sstables_;
  std::optional<MemTable> memtable_;
};

}  // namespace search
