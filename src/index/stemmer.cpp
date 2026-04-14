#include "src/index/stemmer.h"

#include <stdexcept>
#include <utility>

namespace search {

Stemmer::Stemmer(const std::string& language) {
  stemmer_ = sb_stemmer_new(language.c_str(), "UTF_8");
  if (stemmer_ == nullptr) {
    throw std::runtime_error("Failed to create stemmer for language: " + language);
  }
}

Stemmer::~Stemmer() {
  sb_stemmer_delete(stemmer_);
}

std::string Stemmer::Stem(const std::string& word) const {
  const auto* result = sb_stemmer_stem(
      stemmer_,
      reinterpret_cast<const sb_symbol*>(word.data()),
      static_cast<int>(word.size()));

  if (result == nullptr) {
    return word;
  }

  int length = sb_stemmer_length(stemmer_);
  return std::string(reinterpret_cast<const char*>(result), length);
}

}  // namespace search
