#pragma once

#include <map>
#include <optional>

#include "src/lsm/merge_operator.h"
#include "src/lsm/types.h"

namespace search {

class MemTable {
 public:
  enum class Status { kNotFound, kDeleted, kFound };

  void Add(SequenceNumber sequence_number, const Key& key, const Value& value);

  void Delete(SequenceNumber sequence_number, const Key& key);

  Status Get(const Key& key, Value* result, const IMergeOperator* merge_operator = nullptr) const;

  std::vector<std::pair<InternalKey, Value>> ReadRange(std::optional<Key> min, std::optional<Key> max);

  std::vector<std::pair<InternalKey, Value>> Values() const;

  uint64_t Bytes() const { return approx_bytes_; }

 private:
  std::map<InternalKey, Value> entries_;
  uint64_t approx_bytes_ = 0;
};

}  // namespace search
