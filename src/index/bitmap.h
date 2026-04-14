#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "roaring/roaring.hh"

namespace search {

class Bitmap {
 public:
  Bitmap() = default;

  void Add(uint32_t value);
  void Remove(uint32_t value);
  bool Contains(uint32_t value) const;

  uint64_t Cardinality() const;
  bool Empty() const;

  std::vector<uint8_t> Serialize() const;
  static Bitmap Deserialize(const std::vector<uint8_t>& data);

  Bitmap operator&(const Bitmap& other) const;
  Bitmap operator|(const Bitmap& other) const;

  Bitmap& operator&=(const Bitmap& other);
  Bitmap& operator|=(const Bitmap& other);

  bool operator==(const Bitmap& other) const;

 private:
  roaring::Roaring bitmap_;
};

}  // namespace search
