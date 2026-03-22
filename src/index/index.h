#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "src/index/bitmap.h"
#include "src/index/stemmer.h"

namespace search {

using DocumentId = uint32_t;
using Word = std::string;
using Document = std::vector<Word>;

class Index {
 public:
  Index() = default;
  explicit Index(std::shared_ptr<Stemmer> stemmer);

  DocumentId AddDocument(const Document& document);

  Bitmap DocumentsByWord(const Word& word) const;

 private:
  Word NormalizeWord(const Word& word) const;

  DocumentId docs_ = 0;

  std::shared_ptr<Stemmer> stemmer_;
  std::map<Word, Bitmap> index_;
};

}  // namespace search
