#include "src/index/index.h"

#include <utility>

namespace search {

Index::Index(std::shared_ptr<Stemmer> stemmer) : stemmer_(std::move(stemmer)) {}

DocumentId Index::AddDocument(const Document& document) {
  DocumentId id = docs_++;

  for (const Word& word : document) {
    index_[NormalizeWord(word)].Add(id);
  }

  return id;
}

Bitmap Index::DocumentsByWord(const Word& word) const {
  auto it = index_.find(NormalizeWord(word));
  if (it == index_.end()) {
    return Bitmap();
  }

  return it->second;
}

Word Index::NormalizeWord(const Word& word) const {
  if (stemmer_) {
    return stemmer_->Stem(word);
  }
  return word;
}

}  // namespace search
