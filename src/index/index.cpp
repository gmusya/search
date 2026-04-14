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

Bitmap Index::DocumentsByWord(const Word& word) const { return storage_->Get(NormalizeWord(word)); }

Bitmap Index::DocumentsByPrefix(const Word& prefix) const {
  Word max = prefix;
  max.push_back(std::numeric_limits<uint8_t>::max());
  return storage_->GetRange(prefix, max);
}

Word Index::NormalizeWord(const Word& word) const {
  if (stemmer_) {
    return stemmer_->Stem(word);
  }
  return word;
}

////////////////////////////////////////////////////////////////////////////////

KGramIndex::KGramIndex(std::shared_ptr<IStorage> storage, uint64_t k)
    : k_(k), storage_(std::move(storage)) {}

DocumentId KGramIndex::AddDocument(const Document& document) {
  DocumentId id = docs_++;

  for (const Word& word : document) {
    for (uint64_t i = 0; i + k_ - 1 < word.size(); i++) {
      Word kgram = word.substr(i, k_);
      storage_->Add(kgram, id);
    }
  }

  return id;
}

Bitmap KGramIndex::DocumentsByWildcard(const Word& wildcard) const {
  std::optional<Bitmap> total;

  for (uint64_t i = 0; i + k_ - 1 < wildcard.size(); i++) {
    Word search_pattern = wildcard.substr(i, k_);
    Bitmap result = storage_->Get(search_pattern);
    if (!total.has_value()) {
      total = result;
    } else {
      *total &= result;
    }
  }

  return total.value_or(Bitmap()); }
}  // namespace search
