#pragma once

#include <cstdint>
#include <memory>

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

  Bitmap operator&(const Bitmap& other) const;
  Bitmap operator|(const Bitmap& other) const;

  Bitmap& operator&=(const Bitmap& other);
  Bitmap& operator|=(const Bitmap& other);

  bool operator==(const Bitmap& other) const;

 private:
  roaring::Roaring bitmap_;
};

}  // namespace search
