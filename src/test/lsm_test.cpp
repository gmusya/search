#include "src/lsm.h"

#include "gtest/gtest.h"
#include "src/bytes.h"
#include "src/memory_filesystem.h"

namespace search {

TEST(Lsm, Simple) {
  Lsm lsm(std::make_shared<MemoryFileSystem>());

  ASSERT_EQ(lsm.Get(ToBytes(1)), std::nullopt);

  lsm.Insert(ToBytes(1), ToBytes(123));
  ASSERT_EQ(lsm.Get(ToBytes(1)), ToBytes(123));

  lsm.Insert(ToBytes(1), ToBytes(124));
  ASSERT_EQ(lsm.Get(ToBytes(1)), ToBytes(124));

  lsm.Delete(ToBytes(1));
  ASSERT_EQ(lsm.Get(ToBytes(1)), std::nullopt);

  lsm.Insert(ToBytes(1), ToBytes(125));
  ASSERT_EQ(lsm.Get(ToBytes(1)), ToBytes(125));
}

TEST(Lsm, ReadRange) {
  Lsm lsm(std::make_shared<MemoryFileSystem>());

  Key a = ToBytes(1);
  Key b = ToBytes(2);
  Key c = ToBytes(3);
  Key d = ToBytes(4);
  Value v1 = ToBytes(101);
  Value v2 = ToBytes(102);
  Value v3 = ToBytes(103);
  Value v4 = ToBytes(104);

  lsm.Insert(a, v1);
  lsm.Insert(b, v2);
  lsm.Insert(c, v3);
  lsm.Insert(d, v4);
  lsm.Insert(d, v3);
  lsm.Delete(c);
  lsm.Delete(d);

  std::vector<std::pair<Key, Value>> all_values;
  all_values.emplace_back(a, v1);
  all_values.emplace_back(b, v2);
  EXPECT_EQ(lsm.ReadRange(std::nullopt, std::nullopt), all_values);

  std::vector<std::pair<Key, Value>> range_bc;
  range_bc.emplace_back(b, v2);
  EXPECT_EQ(lsm.ReadRange(b, c), range_bc);

  std::vector<std::pair<Key, Value>> range_from_c;
  EXPECT_EQ(lsm.ReadRange(c, std::nullopt), range_from_c);
}

TEST(Lsm, WriteAmplification) {
  Lsm lsm(std::make_shared<MemoryFileSystem>());

#if 0
  uint64_t bytes_written_in_ideal_world = 0;
#endif

  constexpr int kOperations = 6'000;
  for (int i = 0; i < kOperations; ++i) {
    Key key = ToBytes(static_cast<uint64_t>(2 * i));
    Value value = ToBytes(static_cast<uint64_t>(i + 1000));
    lsm.Insert(key, value);

#if 0
    const uint32_t internal_key_size =
        sizeof(InternalKey::sequence_number) + sizeof(InternalKey::is_deleted) + sizeof(uint32_t) + key.size();
    const uint32_t value_size = sizeof(uint32_t) + value.size();

    bytes_written_in_ideal_world += internal_key_size + value_size;
#endif
  }

  for (int i = 0; i < kOperations; ++i) {
    Key key = ToBytes(static_cast<uint64_t>(2 * i));
    Value value = ToBytes(static_cast<uint64_t>(i + 1000));

    ASSERT_EQ(lsm.Get(key), value);
  }

  for (int i = 0; i < kOperations; ++i) {
    Key key = ToBytes(static_cast<uint64_t>(2 * i + 1));

    ASSERT_EQ(lsm.Get(key), std::nullopt);
  }
}

}  // namespace search
