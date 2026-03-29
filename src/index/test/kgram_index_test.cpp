#include "src/index/index.h"

#include <memory>

#include "gtest/gtest.h"

namespace search {

class MapStorage : public IStorage {
 public:
  void Add(const Word& word, DocumentId document_id) override { index_[word].Add(document_id); }

  Bitmap Get(const Word& word) const override {
    auto it = index_.find(word);
    if (it == index_.end()) {
      return Bitmap();
    }
    return it->second;
  }

  Bitmap GetRange(const Word& min, const Word& max) const override {
    auto it = index_.lower_bound(min);
    auto end = index_.upper_bound(max);
    Bitmap bitmap;
    for (; it != end; ++it) {
      bitmap |= it->second;
    }
    return bitmap;
  }

 private:
  std::map<Word, Bitmap> index_;
};

TEST(KGramIndex, Simple) {
  KGramIndex index(std::make_shared<MapStorage>(), 3);
  index.AddDocument({"fabcde"});
  index.AddDocument({"abcxyz"});
  index.AddDocument({"xyzdef"});

  Bitmap result = index.DocumentsByWildcard("abcd");
  EXPECT_EQ(result.Cardinality(), 1);
  EXPECT_TRUE(result.Contains(0));
}

TEST(KGramIndex, FalsePositives) {
  KGramIndex index(std::make_shared<MapStorage>(), 3);
  index.AddDocument({"abcdefgh"});
  index.AddDocument({"abcxxbcd"});

  Bitmap result = index.DocumentsByWildcard("abcd");
  EXPECT_EQ(result.Cardinality(), 2);
  EXPECT_TRUE(result.Contains(0));
  EXPECT_TRUE(result.Contains(1));
}

TEST(KGramIndex, ShortWords) {
  KGramIndex index(std::make_shared<MapStorage>(), 3);
  index.AddDocument({"hi"});
  index.AddDocument({"hello"});
  index.AddDocument({""});

  Bitmap result = index.DocumentsByWildcard("hel");
  EXPECT_EQ(result.Cardinality(), 1);
  EXPECT_TRUE(result.Contains(1));
  EXPECT_FALSE(result.Contains(0));
}

}  // namespace search
