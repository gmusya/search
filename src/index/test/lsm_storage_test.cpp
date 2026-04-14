#include "src/index/lsm_storage.h"

#include <memory>

#include "gtest/gtest.h"
#include "src/index/index.h"
#include "src/lsm/memory_filesystem.h"

namespace search {

TEST(LsmStorage, BasicAddAndGet) {
  auto storage = std::make_shared<LsmStorage>(std::make_shared<MemoryFileSystem>());

  storage->Add("cat", 0);
  storage->Add("cat", 2);
  storage->Add("dog", 1);

  Bitmap cat_docs = storage->Get("cat");
  EXPECT_EQ(cat_docs.Cardinality(), 2);
  EXPECT_TRUE(cat_docs.Contains(0));
  EXPECT_TRUE(cat_docs.Contains(2));

  Bitmap dog_docs = storage->Get("dog");
  EXPECT_EQ(dog_docs.Cardinality(), 1);
  EXPECT_TRUE(dog_docs.Contains(1));

  Bitmap missing = storage->Get("bird");
  EXPECT_TRUE(missing.Empty());
}

TEST(LsmStorage, MergeAcrossFlushes) {
  auto storage =
      std::make_shared<LsmStorage>(std::make_shared<MemoryFileSystem>(), Lsm::Parameters{.memtable_bytes_limit = 64});

  for (uint32_t i = 0; i < 100; ++i) {
    storage->Add("word", i);
  }

  Bitmap result = storage->Get("word");
  EXPECT_EQ(result.Cardinality(), 100);
  for (uint32_t i = 0; i < 100; ++i) {
    EXPECT_TRUE(result.Contains(i));
  }
}

TEST(LsmStorage, MultipleWordsAcrossFlushes) {
  auto storage =
      std::make_shared<LsmStorage>(std::make_shared<MemoryFileSystem>(), Lsm::Parameters{.memtable_bytes_limit = 128});

  for (uint32_t i = 0; i < 50; ++i) {
    storage->Add("alpha", i);
    storage->Add("beta", i * 2);
  }

  Bitmap alpha = storage->Get("alpha");
  EXPECT_EQ(alpha.Cardinality(), 50);

  Bitmap beta = storage->Get("beta");
  for (uint32_t i = 0; i < 50; ++i) {
    EXPECT_TRUE(beta.Contains(i * 2));
  }
}

TEST(LsmStorage, WithIndex) {
  auto storage =
      std::make_shared<LsmStorage>(std::make_shared<MemoryFileSystem>(), Lsm::Parameters{.memtable_bytes_limit = 128});

  Index index(storage);

  index.AddDocument({"cat", "dog"});
  index.AddDocument({"dog", "bird"});
  index.AddDocument({"cat", "bird"});

  Bitmap cat_docs = index.DocumentsByWord("cat");
  EXPECT_EQ(cat_docs.Cardinality(), 2);
  EXPECT_TRUE(cat_docs.Contains(0));
  EXPECT_TRUE(cat_docs.Contains(2));

  Bitmap dog_docs = index.DocumentsByWord("dog");
  EXPECT_EQ(dog_docs.Cardinality(), 2);
  EXPECT_TRUE(dog_docs.Contains(0));
  EXPECT_TRUE(dog_docs.Contains(1));

  Bitmap bird_docs = index.DocumentsByWord("bird");
  EXPECT_EQ(bird_docs.Cardinality(), 2);
  EXPECT_TRUE(bird_docs.Contains(1));
  EXPECT_TRUE(bird_docs.Contains(2));
}

TEST(LsmStorage, WithIndexManyDocuments) {
  auto storage =
      std::make_shared<LsmStorage>(std::make_shared<MemoryFileSystem>(), Lsm::Parameters{.memtable_bytes_limit = 256});

  Index index(storage);

  for (uint32_t i = 0; i < 200; ++i) {
    if (i % 2 == 0) {
      index.AddDocument({"even", "all"});
    } else {
      index.AddDocument({"odd", "all"});
    }
  }

  Bitmap even_docs = index.DocumentsByWord("even");
  EXPECT_EQ(even_docs.Cardinality(), 100);

  Bitmap odd_docs = index.DocumentsByWord("odd");
  EXPECT_EQ(odd_docs.Cardinality(), 100);

  Bitmap all_docs = index.DocumentsByWord("all");
  EXPECT_EQ(all_docs.Cardinality(), 200);
}

}  // namespace search
