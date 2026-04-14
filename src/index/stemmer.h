#pragma once

#include <string>

#include "libstemmer.h"

namespace search {

class Stemmer {
 public:
  explicit Stemmer(const std::string& language);
  ~Stemmer();

  Stemmer(const Stemmer&) = delete;
  Stemmer& operator=(const Stemmer&) = delete;

  Stemmer(Stemmer&& other) = delete;
  Stemmer& operator=(Stemmer&& other) = delete;

  std::string Stem(const std::string& word) const;

 private:
  sb_stemmer* stemmer_ = nullptr;
};

}  // namespace search
