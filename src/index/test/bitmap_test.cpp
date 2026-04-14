#include "src/index/bitmap.h"

#include "gtest/gtest.h"

namespace search {

TEST(Bitmap, AddAndContains) {
  Bitmap bitmap;
  EXPECT_TRUE(bitmap.Empty());
  EXPECT_EQ(bitmap.Cardinality(), 0);

  bitmap.Add(1);
  bitmap.Add(5);
  bitmap.Add(100);

  EXPECT_FALSE(bitmap.Empty());
  EXPECT_EQ(bitmap.Cardinality(), 3);

  EXPECT_TRUE(bitmap.Contains(1));
  EXPECT_TRUE(bitmap.Contains(5));
  EXPECT_TRUE(bitmap.Contains(100));
  EXPECT_FALSE(bitmap.Contains(2));
  EXPECT_FALSE(bitmap.Contains(99));
}

TEST(Bitmap, Remove) {
  Bitmap bitmap;
  bitmap.Add(10);
  bitmap.Add(20);
  bitmap.Add(30);

  EXPECT_TRUE(bitmap.Contains(20));
  bitmap.Remove(20);
  EXPECT_FALSE(bitmap.Contains(20));
  EXPECT_EQ(bitmap.Cardinality(), 2);
}

TEST(Bitmap, Intersection) {
  Bitmap a;
  a.Add(1);
  a.Add(2);
  a.Add(3);

  Bitmap b;
  b.Add(2);
  b.Add(3);
  b.Add(4);

  Bitmap c = a & b;
  EXPECT_EQ(c.Cardinality(), 2);
  EXPECT_FALSE(c.Contains(1));
  EXPECT_TRUE(c.Contains(2));
  EXPECT_TRUE(c.Contains(3));
  EXPECT_FALSE(c.Contains(4));
}

TEST(Bitmap, Union) {
  Bitmap a;
  a.Add(1);
  a.Add(2);

  Bitmap b;
  b.Add(3);
  b.Add(4);

  Bitmap c = a | b;
  EXPECT_EQ(c.Cardinality(), 4);
  EXPECT_TRUE(c.Contains(1));
  EXPECT_TRUE(c.Contains(2));
  EXPECT_TRUE(c.Contains(3));
  EXPECT_TRUE(c.Contains(4));
}

TEST(Bitmap, CompoundAssignment) {
  Bitmap a;
  a.Add(1);
  a.Add(2);
  a.Add(3);

  Bitmap b;
  b.Add(2);
  b.Add(3);
  b.Add(4);

  Bitmap a_copy = a;
  a_copy &= b;
  EXPECT_EQ(a_copy.Cardinality(), 2);
  EXPECT_TRUE(a_copy.Contains(2));
  EXPECT_TRUE(a_copy.Contains(3));

  Bitmap b_copy = b;
  b_copy |= a;
  EXPECT_EQ(b_copy.Cardinality(), 4);
}

TEST(Bitmap, Equality) {
  Bitmap a;
  a.Add(1);
  a.Add(2);

  Bitmap b;
  b.Add(1);
  b.Add(2);

  EXPECT_TRUE(a == b);

  b.Add(3);
  EXPECT_FALSE(a == b);
}

}  // namespace search
