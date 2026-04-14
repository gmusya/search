#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "src/index/bitmap.h"
#include "src/index/stemmer.h"
#include "src/lsm/macro.h"

namespace search {

using DocumentId = uint32_t;
using Word = std::string;
using Document = std::vector<Word>;

class IStorage {
 public:
  virtual void Add(const Word& word, DocumentId document_id) = 0;
  virtual Bitmap Get(const Word& word) const = 0;
  virtual Bitmap GetRange(const Word& min, const Word& max) const = 0;

  virtual ~IStorage() = default;
};

class Index {
 public:
  Index(std::shared_ptr<IStorage> storage, std::shared_ptr<Stemmer> stemmer = nullptr);

  DocumentId AddDocument(const Document& document);

  Bitmap DocumentsByWord(const Word& word) const;

  Bitmap DocumentsByPrefix(const Word& prefix) const;

 private:
  Word NormalizeWord(const Word& word) const;

  DocumentId docs_ = 0;

  std::shared_ptr<Stemmer> stemmer_;
  std::shared_ptr<IStorage> storage_;
};

class KGramIndex {
 public:
  KGramIndex(std::shared_ptr<IStorage> storage, uint64_t k);

  DocumentId AddDocument(const Document& document);

  Bitmap DocumentsByWildcard(const Word& word) const;

 private:
  uint64_t k_ = 0;
  DocumentId docs_ = 0;

  std::shared_ptr<IStorage> storage_;
};

}  // namespace search
