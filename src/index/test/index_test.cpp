#include "src/index/index.h"

#include <memory>

#include "gtest/gtest.h"

namespace search {

TEST(Index, DocumentsByWord) {
  Index index;
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

TEST(Index, DocumentsByWordMissing) {
  Index index;
  index.AddDocument({"hello"});

  Bitmap result = index.DocumentsByWord("nonexistent");
  EXPECT_TRUE(result.Empty());
  EXPECT_EQ(result.Cardinality(), 0);
}

TEST(Index, DuplicateWordsInDocument) {
  Index index;
  index.AddDocument({"hello", "hello", "world"});

  Bitmap hello_docs = index.DocumentsByWord("hello");
  EXPECT_EQ(hello_docs.Cardinality(), 1);
  EXPECT_TRUE(hello_docs.Contains(0));
}

TEST(Index, WithStemmer) {
  auto stemmer = std::make_shared<Stemmer>("english");
  Index index(stemmer);

  // "running" and "runs" should both stem to the same root
  index.AddDocument({"running", "fast"});  // 0
  index.AddDocument({"runs", "slow"});     // 1
  index.AddDocument({"jumped", "high"});   // 2

  // Searching for any form should find both documents with run-variants
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

  // "jumped" should only match doc 2
  result = index.DocumentsByWord("jumped");
  EXPECT_EQ(result.Cardinality(), 1);
  EXPECT_TRUE(result.Contains(2));

  // "jumping" should also match doc 2 (same stem)
  result = index.DocumentsByWord("jumping");
  EXPECT_EQ(result.Cardinality(), 1);
  EXPECT_TRUE(result.Contains(2));
}

}  // namespace search
