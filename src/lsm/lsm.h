#pragma once

#include <algorithm>
#include <iostream>
#include <iterator>
#include <optional>
#include <vector>

#include "src/lsm/bloom_filter.h"
#include "src/lsm/file.h"
#include "src/lsm/filesystem.h"
#include "src/lsm/memtable.h"
#include "src/lsm/sstable.h"
#include "src/lsm/types.h"

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

  std::optional<Value> Get(const Key& key);

  void Insert(const Key& key, const Value& value);

  void Delete(const Key& key);

  std::vector<std::pair<Key, Value>> ReadRange(std::optional<Key> min, std::optional<Key> max);

 private:
  void FlushMemTable();

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
