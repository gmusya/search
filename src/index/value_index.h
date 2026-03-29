#pragma once

#include <cstdint>
#include <vector>

#include "src/index/bitmap.h"

namespace search {

using DocumentId = uint32_t;
using Word = std::string;
using Document = std::vector<Word>;

class ValueIndex {
 public:
  ValueIndex();

  DocumentId AddDocument(uint64_t value);

  Bitmap DocumentsByLessThan(uint64_t value) const;
  Bitmap DocumentsByInterval(uint64_t min, uint64_t max) const;

 private:
  static constexpr int kBits = 64;

  DocumentId docs_ = 0;
  Bitmap all_docs_;
  std::vector<Bitmap> contains_bit_;
};

}  // namespace search
