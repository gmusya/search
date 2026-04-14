#include "src/index/value_index.h"

#include "gtest/gtest.h"

namespace search {

TEST(ValueIndex, LessThanSimple) {
  ValueIndex index;
  index.AddDocument(10);
  index.AddDocument(20);
  index.AddDocument(30);
  index.AddDocument(5);

  Bitmap result = index.DocumentsByLessThan(15);
  EXPECT_EQ(result.Cardinality(), 2);
  EXPECT_TRUE(result.Contains(0));
  EXPECT_FALSE(result.Contains(1));
  EXPECT_FALSE(result.Contains(2));
  EXPECT_TRUE(result.Contains(3));
}

TEST(ValueIndex, LessThanNone) {
  ValueIndex index;
  index.AddDocument(10);
  index.AddDocument(20);

  Bitmap result = index.DocumentsByLessThan(5);
  EXPECT_TRUE(result.Empty());
}

TEST(ValueIndex, LessThanAll) {
  ValueIndex index;
  index.AddDocument(1);
  index.AddDocument(2);
  index.AddDocument(3);

  Bitmap result = index.DocumentsByLessThan(100);
  EXPECT_EQ(result.Cardinality(), 3);
}

TEST(ValueIndex, LessThanBoundary) {
  ValueIndex index;
  index.AddDocument(10);
  index.AddDocument(10);
  index.AddDocument(11);

  Bitmap result = index.DocumentsByLessThan(10);
  EXPECT_TRUE(result.Empty());

  result = index.DocumentsByLessThan(11);
  EXPECT_EQ(result.Cardinality(), 2);
  EXPECT_TRUE(result.Contains(0));
  EXPECT_TRUE(result.Contains(1));
  EXPECT_FALSE(result.Contains(2));
}

TEST(ValueIndex, IntervalSimple) {
  ValueIndex index;
  index.AddDocument(5);
  index.AddDocument(10);
  index.AddDocument(15);
  index.AddDocument(20);
  index.AddDocument(25);

  Bitmap result = index.DocumentsByInterval(10, 21);
  EXPECT_EQ(result.Cardinality(), 3);
  EXPECT_FALSE(result.Contains(0));
  EXPECT_TRUE(result.Contains(1));
  EXPECT_TRUE(result.Contains(2));
  EXPECT_TRUE(result.Contains(3));
  EXPECT_FALSE(result.Contains(4));
}

TEST(ValueIndex, IntervalEmpty) {
  ValueIndex index;
  index.AddDocument(5);
  index.AddDocument(20);

  Bitmap result = index.DocumentsByInterval(10, 15);
  EXPECT_TRUE(result.Empty());
}

}  // namespace search
