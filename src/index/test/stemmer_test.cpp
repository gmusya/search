#include "src/index/stemmer.h"

#include <stdexcept>

#include "gtest/gtest.h"

namespace search {

TEST(Stemmer, EnglishBasic) {
  Stemmer stemmer("english");

  EXPECT_EQ(stemmer.Stem("running"), "run");
  EXPECT_EQ(stemmer.Stem("runs"), "run");
  EXPECT_EQ(stemmer.Stem("ran"), "ran");
  EXPECT_EQ(stemmer.Stem("run"), "run");
}

TEST(Stemmer, EnglishSuffixes) {
  Stemmer stemmer("english");

  EXPECT_EQ(stemmer.Stem("connected"), "connect");
  EXPECT_EQ(stemmer.Stem("connecting"), "connect");
  EXPECT_EQ(stemmer.Stem("connection"), "connect");
  EXPECT_EQ(stemmer.Stem("connections"), "connect");
}

TEST(Stemmer, EnglishPlurals) {
  Stemmer stemmer("english");

  EXPECT_EQ(stemmer.Stem("cats"), "cat");
  EXPECT_EQ(stemmer.Stem("cat"), "cat");
  EXPECT_EQ(stemmer.Stem("dogs"), "dog");
  EXPECT_EQ(stemmer.Stem("dog"), "dog");
}

TEST(Stemmer, EnglishAlreadyStemmed) {
  Stemmer stemmer("english");

  EXPECT_EQ(stemmer.Stem("big"), "big");
  EXPECT_EQ(stemmer.Stem("red"), "red");
}

TEST(Stemmer, Russian) {
  Stemmer stemmer("russian");

  std::string stem_cat = stemmer.Stem("кошка");
  EXPECT_EQ(stemmer.Stem("кошки"), stem_cat);
  EXPECT_EQ(stemmer.Stem("кошке"), stem_cat);
  EXPECT_EQ(stemmer.Stem("кошку"), stem_cat);
}

TEST(Stemmer, InvalidLanguage) {
  EXPECT_THROW(Stemmer("nonexistent_language"), std::runtime_error);
}

TEST(Stemmer, EmptyString) {
  Stemmer stemmer("english");

  EXPECT_EQ(stemmer.Stem(""), "");
}

TEST(Stemmer, MultipleCallsSameInstance) {
  Stemmer stemmer("english");

  EXPECT_EQ(stemmer.Stem("running"), "run");
  EXPECT_EQ(stemmer.Stem("cats"), "cat");
  EXPECT_EQ(stemmer.Stem("running"), "run");
  EXPECT_EQ(stemmer.Stem("dogs"), "dog");
}

}  // namespace search
