#pragma once

#include "src/lsm/types.h"

namespace search {

class IMergeOperator {
 public:
  virtual Value Merge(const Value& existing, const Value& update) const = 0;

  virtual ~IMergeOperator() = default;
};

}  // namespace search
