#include "src/index/bitmap.h"

namespace search {

void Bitmap::Add(uint32_t value) {
  bitmap_.add(value);
}

void Bitmap::Remove(uint32_t value) {
  bitmap_.remove(value);
}

bool Bitmap::Contains(uint32_t value) const {
  return bitmap_.contains(value);
}

uint64_t Bitmap::Cardinality() const {
  return bitmap_.cardinality();
}

bool Bitmap::Empty() const {
  return bitmap_.isEmpty();
}

std::vector<uint8_t> Bitmap::Serialize() const {
  size_t size = bitmap_.getSizeInBytes();
  std::vector<uint8_t> data(size);
  bitmap_.write(reinterpret_cast<char*>(data.data()));
  return data;
}

Bitmap Bitmap::Deserialize(const std::vector<uint8_t>& data) {
  Bitmap result;
  result.bitmap_ = roaring::Roaring::read(reinterpret_cast<const char*>(data.data()));
  return result;
}

Bitmap Bitmap::operator&(const Bitmap& other) const {
  Bitmap result;
  result.bitmap_ = bitmap_ & other.bitmap_;
  return result;
}

Bitmap Bitmap::operator|(const Bitmap& other) const {
  Bitmap result;
  result.bitmap_ = bitmap_ | other.bitmap_;
  return result;
}

Bitmap& Bitmap::operator&=(const Bitmap& other) {
  bitmap_ &= other.bitmap_;
  return *this;
}

Bitmap& Bitmap::operator|=(const Bitmap& other) {
  bitmap_ |= other.bitmap_;
  return *this;
}

bool Bitmap::operator==(const Bitmap& other) const {
  return bitmap_ == other.bitmap_;
}

}  // namespace search
