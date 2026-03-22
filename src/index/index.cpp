#include "src/index/index.h"

#include <utility>

namespace search {

Index::Index(std::shared_ptr<IStorage> storage, std::shared_ptr<Stemmer> stemmer)
    : stemmer_(std::move(stemmer)), storage_(std::move(storage)) {}

DocumentId Index::AddDocument(const Document& document) {
  DocumentId id = docs_++;

  for (const Word& word : document) {
    storage_->Add(NormalizeWord(word), id);
  }

  return id;
}

Bitmap Index::DocumentsByWord(const Word& word) const {
  return storage_->Get(NormalizeWord(word));
}

Word Index::NormalizeWord(const Word& word) const {
  if (stemmer_) {
    return stemmer_->Stem(word);
  }
  return word;
}

}  // namespace search
