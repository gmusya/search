#include "src/memtable.h"

#include "gtest/gtest.h"
#include "src/bytes.h"

namespace search {

TEST(MemTable, Simple) {
  MemTable table;
  uint64_t sequence_number = 0;

  Key key = ToBytes(1);

  Value output;
  EXPECT_EQ(table.Get(key, &output), MemTable::Status::kNotFound);

  Value v1 = ToBytes(101);
  table.Add(++sequence_number, key, v1);
  EXPECT_EQ(table.Get(key, &output), MemTable::Status::kFound);
  EXPECT_EQ(output, v1);

  Value v2 = ToBytes(102);
  table.Add(++sequence_number, key, v2);
  EXPECT_EQ(table.Get(key, &output), MemTable::Status::kFound);
  EXPECT_EQ(output, v2);

  table.Delete(++sequence_number, key);
  EXPECT_EQ(table.Get(key, &output), MemTable::Status::kDeleted);
}

TEST(MemTable, ReadRange) {
  MemTable table;

  Key a = ToBytes(1);
  Key b = ToBytes(2);
  Key c = ToBytes(3);
  Key d = ToBytes(4);
  Value v1 = ToBytes(101);
  Value v2 = ToBytes(102);
  Value v3 = ToBytes(103);
  Value v4 = ToBytes(104);

  table.Add(5, a, v1);
  table.Add(4, b, v2);
  table.Delete(3, c);
  table.Add(2, d, v4);
  table.Add(1, d, v3);

  std::vector<std::pair<InternalKey, Value>> values;
  values.emplace_back(InternalKey{.key = a, .sequence_number = 5, .is_deleted = false}, v1);
  values.emplace_back(InternalKey{.key = b, .sequence_number = 4, .is_deleted = false}, v2);
  values.emplace_back(InternalKey{.key = c, .sequence_number = 3, .is_deleted = true}, Bytes{});
  values.emplace_back(InternalKey{.key = d, .sequence_number = 2, .is_deleted = false}, v4);
  values.emplace_back(InternalKey{.key = d, .sequence_number = 1, .is_deleted = false}, v3);

  EXPECT_EQ(table.ReadRange(std::nullopt, std::nullopt), values);

  std::vector<std::pair<InternalKey, Value>> range_bc;
  range_bc.emplace_back(InternalKey{.key = b, .sequence_number = 4, .is_deleted = false}, v2);
  range_bc.emplace_back(InternalKey{.key = c, .sequence_number = 3, .is_deleted = true}, Bytes{});
  EXPECT_EQ(table.ReadRange(b, c), range_bc);

  std::vector<std::pair<InternalKey, Value>> range_from_c;
  range_from_c.emplace_back(InternalKey{.key = c, .sequence_number = 3, .is_deleted = true}, Bytes{});
  range_from_c.emplace_back(InternalKey{.key = d, .sequence_number = 2, .is_deleted = false}, v4);
  range_from_c.emplace_back(InternalKey{.key = d, .sequence_number = 1, .is_deleted = false}, v3);
  EXPECT_EQ(table.ReadRange(c, std::nullopt), range_from_c);
}

}  // namespace search
