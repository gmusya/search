#include "src/index/value_index.h"

namespace search {

ValueIndex::ValueIndex() { contains_bit_.resize(kBits); }

DocumentId ValueIndex::AddDocument(uint64_t value) {
  DocumentId id = docs_++;
  all_docs_.Add(id);
  for (int i = 0; i < kBits; i++) {
    if (value & (1ull << i)) {
      contains_bit_[i].Add(id);
    }
  }
  return id;
}

Bitmap ValueIndex::DocumentsByInterval(uint64_t min, uint64_t max) const {
  return DocumentsByLessThan(max) - DocumentsByLessThan(min);
}

Bitmap ValueIndex::DocumentsByLessThan(uint64_t value) const {
  Bitmap still_equal = all_docs_;
  Bitmap result;

  for (int i = kBits - 1; i >= 0; i--) {
    if (value & (1ull << i)) {
      result |= (still_equal - contains_bit_[i]);
      still_equal &= contains_bit_[i];
    } else {
      still_equal -= contains_bit_[i];
    }
  }

  return result;
}

}  // namespace search
