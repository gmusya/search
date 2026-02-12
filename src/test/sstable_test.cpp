#include "src/sstable.h"

#include "gtest/gtest.h"
#include "src/memory_file.h"
#include "src/types.h"

namespace search {

TEST(SSTable, PointLookup) {
  auto file = std::make_shared<MemoryFile>();

  Key a = ToBytes(1);
  Key b = ToBytes(2);
  Key c = ToBytes(3);
  Value v1 = ToBytes(101);
  Value v2 = ToBytes(102);
  Value v3 = ToBytes(103);

  {
    SSTableWriter builder(file);

    std::vector<std::pair<InternalKey, Value>> values;
    values.emplace_back(InternalKey{.key = a, .sequence_number = 5, .is_deleted = false}, v3);
    values.emplace_back(InternalKey{.key = a, .sequence_number = 2, .is_deleted = true}, Bytes{});
    values.emplace_back(InternalKey{.key = a, .sequence_number = 1, .is_deleted = false}, v1);
    values.emplace_back(InternalKey{.key = b, .sequence_number = 6, .is_deleted = true}, Bytes{});
    values.emplace_back(InternalKey{.key = b, .sequence_number = 4, .is_deleted = false}, v2);

    std::move(builder).Write(values);
  }

  SSTableReader reader(file);

  {
    Value value;

    EXPECT_EQ(reader.Get(a, &value), SSTableReader::Status::kFound);
    EXPECT_EQ(value, v3);
  }
  {
    Value value;
    EXPECT_EQ(reader.Get(b, &value), SSTableReader::Status::kDeleted);
  }
  {
    Value value;
    EXPECT_EQ(reader.Get(c, &value), SSTableReader::Status::kNotFound);
  }
}

TEST(SSTable, ReadRange) {
  auto file = std::make_shared<MemoryFile>();

  Key a = ToBytes(1);
  Key b = ToBytes(2);
  Key c = ToBytes(3);
  Key d = ToBytes(4);
  Value v1 = ToBytes(101);
  Value v2 = ToBytes(102);
  Value v3 = ToBytes(103);
  Value v4 = ToBytes(104);

  std::vector<std::pair<InternalKey, Value>> values;
  values.emplace_back(InternalKey{.key = a, .sequence_number = 5, .is_deleted = false}, v1);
  values.emplace_back(InternalKey{.key = b, .sequence_number = 4, .is_deleted = false}, v2);
  values.emplace_back(InternalKey{.key = c, .sequence_number = 3, .is_deleted = true}, Bytes{});
  values.emplace_back(InternalKey{.key = d, .sequence_number = 2, .is_deleted = false}, v4);
  values.emplace_back(InternalKey{.key = d, .sequence_number = 1, .is_deleted = false}, v3);

  {
    SSTableWriter builder(file);
    std::move(builder).Write(values);
  }

  SSTableReader reader(file);

  EXPECT_EQ(reader.ReadRange(std::nullopt, std::nullopt), values);

  std::vector<std::pair<InternalKey, Value>> range_bc;
  range_bc.emplace_back(InternalKey{.key = b, .sequence_number = 4, .is_deleted = false}, v2);
  range_bc.emplace_back(InternalKey{.key = c, .sequence_number = 3, .is_deleted = true}, Bytes{});
  EXPECT_EQ(reader.ReadRange(b, c), range_bc);

  std::vector<std::pair<InternalKey, Value>> range_from_c;
  range_from_c.emplace_back(InternalKey{.key = c, .sequence_number = 3, .is_deleted = true}, Bytes{});
  range_from_c.emplace_back(InternalKey{.key = d, .sequence_number = 2, .is_deleted = false}, v4);
  range_from_c.emplace_back(InternalKey{.key = d, .sequence_number = 1, .is_deleted = false}, v3);
  EXPECT_EQ(reader.ReadRange(c, std::nullopt), range_from_c);
}

}  // namespace search
