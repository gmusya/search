#include "src/index/index.h"

#include <memory>

#include "gtest/gtest.h"
#include "src/index/lsm_storage.h"
#include "src/lsm/memory_filesystem.h"

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

std::shared_ptr<IStorage> MakeMapStorage() { return std::make_shared<MapStorage>(); }

std::shared_ptr<IStorage> MakeLsmStorage() {
  return std::make_shared<LsmStorage>(std::make_shared<MemoryFileSystem>());
}

class IndexTest : public ::testing::TestWithParam<std::function<std::shared_ptr<IStorage>()>> {};

TEST_P(IndexTest, DocumentsByWord) {
  Index index(GetParam()());
  index.AddDocument({"cat", "dog"});
  index.AddDocument({"dog", "bird"});
  index.AddDocument({"cat", "bird"});

  Bitmap cat_docs = index.DocumentsByWord("cat");
  EXPECT_EQ(cat_docs.Cardinality(), 2);
  EXPECT_TRUE(cat_docs.Contains(0));
  EXPECT_FALSE(cat_docs.Contains(1));
  EXPECT_TRUE(cat_docs.Contains(2));

  Bitmap dog_docs = index.DocumentsByWord("dog");
  EXPECT_EQ(dog_docs.Cardinality(), 2);
  EXPECT_TRUE(dog_docs.Contains(0));
  EXPECT_TRUE(dog_docs.Contains(1));
  EXPECT_FALSE(dog_docs.Contains(2));

  Bitmap bird_docs = index.DocumentsByWord("bird");
  EXPECT_EQ(bird_docs.Cardinality(), 2);
  EXPECT_FALSE(bird_docs.Contains(0));
  EXPECT_TRUE(bird_docs.Contains(1));
  EXPECT_TRUE(bird_docs.Contains(2));
}

TEST_P(IndexTest, DocumentsByWordMissing) {
  Index index(GetParam()());
  index.AddDocument({"hello"});

  Bitmap result = index.DocumentsByWord("nonexistent");
  EXPECT_TRUE(result.Empty());
  EXPECT_EQ(result.Cardinality(), 0);
}

TEST_P(IndexTest, DuplicateWordsInDocument) {
  Index index(GetParam()());
  index.AddDocument({"hello", "hello", "world"});

  Bitmap hello_docs = index.DocumentsByWord("hello");
  EXPECT_EQ(hello_docs.Cardinality(), 1);
  EXPECT_TRUE(hello_docs.Contains(0));
}

TEST_P(IndexTest, WithStemmer) {
  auto stemmer = std::make_shared<Stemmer>("english");
  Index index(GetParam()(), stemmer);

  index.AddDocument({"running", "fast"});
  index.AddDocument({"runs", "slow"});
  index.AddDocument({"jumped", "high"});

  Bitmap result = index.DocumentsByWord("running");
  EXPECT_EQ(result.Cardinality(), 2);
  EXPECT_TRUE(result.Contains(0));
  EXPECT_TRUE(result.Contains(1));

  result = index.DocumentsByWord("runs");
  EXPECT_EQ(result.Cardinality(), 2);
  EXPECT_TRUE(result.Contains(0));
  EXPECT_TRUE(result.Contains(1));

  result = index.DocumentsByWord("run");
  EXPECT_EQ(result.Cardinality(), 2);
  EXPECT_TRUE(result.Contains(0));
  EXPECT_TRUE(result.Contains(1));

  result = index.DocumentsByWord("jumped");
  EXPECT_EQ(result.Cardinality(), 1);
  EXPECT_TRUE(result.Contains(2));

  result = index.DocumentsByWord("jumping");
  EXPECT_EQ(result.Cardinality(), 1);
  EXPECT_TRUE(result.Contains(2));
}

TEST_P(IndexTest, DocumentsByPrefix) {
  Index index(GetParam()());
  index.AddDocument({"cat", "car"});
  index.AddDocument({"dog", "dove"});
  index.AddDocument({"catalog", "bird"});

  Bitmap ca_docs = index.DocumentsByPrefix("ca");
  EXPECT_EQ(ca_docs.Cardinality(), 2);
  EXPECT_TRUE(ca_docs.Contains(0));
  EXPECT_TRUE(ca_docs.Contains(2));

  Bitmap cat_docs = index.DocumentsByPrefix("cat");
  EXPECT_EQ(cat_docs.Cardinality(), 2);
  EXPECT_TRUE(cat_docs.Contains(0));
  EXPECT_TRUE(cat_docs.Contains(2));

  Bitmap do_docs = index.DocumentsByPrefix("do");
  EXPECT_EQ(do_docs.Cardinality(), 1);
  EXPECT_TRUE(do_docs.Contains(1));

  Bitmap dog_docs = index.DocumentsByPrefix("dog");
  EXPECT_EQ(dog_docs.Cardinality(), 1);
  EXPECT_TRUE(dog_docs.Contains(1));
}

TEST_P(IndexTest, DocumentsByPrefixMissing) {
  Index index(GetParam()());
  index.AddDocument({"hello", "world"});

  Bitmap result = index.DocumentsByPrefix("xyz");
  EXPECT_TRUE(result.Empty());
}

TEST_P(IndexTest, DocumentsByPrefixFullWord) {
  Index index(GetParam()());
  index.AddDocument({"apple"});
  index.AddDocument({"application"});

  Bitmap result = index.DocumentsByPrefix("apple");
  EXPECT_EQ(result.Cardinality(), 1);
  EXPECT_TRUE(result.Contains(0));

  result = index.DocumentsByPrefix("app");
  EXPECT_EQ(result.Cardinality(), 2);
  EXPECT_TRUE(result.Contains(0));
  EXPECT_TRUE(result.Contains(1));
}

TEST_P(IndexTest, DocumentsByPrefixEmptyPrefix) {
  Index index(GetParam()());
  index.AddDocument({"cat"});
  index.AddDocument({"dog"});

  Bitmap result = index.DocumentsByPrefix("");
  EXPECT_EQ(result.Cardinality(), 2);
  EXPECT_TRUE(result.Contains(0));
  EXPECT_TRUE(result.Contains(1));
}

INSTANTIATE_TEST_SUITE_P(MapStorage, IndexTest, ::testing::Values(MakeMapStorage));
INSTANTIATE_TEST_SUITE_P(LsmStorage, IndexTest, ::testing::Values(MakeLsmStorage));

}  // namespace search
