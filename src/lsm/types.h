#pragma once

#include "src/lsm/bytes.h"

namespace search {

using SequenceNumber = uint64_t;

using Key = Bytes;
using Value = Bytes;

struct InternalKey {
  Key key;
  SequenceNumber sequence_number;
  bool is_deleted;

  std::strong_ordering operator<=>(const InternalKey& other) const {
    {
      std::strong_ordering key_cmp = key <=> other.key;
      if (key_cmp != std::strong_ordering::equal) {
        return key_cmp;
      }
    }
    {
      std::strong_ordering seq_cmp = other.sequence_number <=> sequence_number;
      if (seq_cmp != std::strong_ordering::equal) {
        return seq_cmp;
      }
    }
    {
      std::strong_ordering type_cmp = is_deleted <=> other.is_deleted;
      return type_cmp;
    }
  }

  bool operator==(const InternalKey& other) const = default;
};

}  // namespace search
