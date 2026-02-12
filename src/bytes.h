#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <vector>

#include "src/assert.h"

namespace search {

using Bytes = std::vector<uint8_t>;

template <typename T>
Bytes ToBytes(const T& value);

template <typename T>
T FromBytes(const Bytes& bytes);

////////////////////////////////////////////////////////////////////////////////

template <typename T>
  requires(std::is_trivially_copyable_v<T>)
inline Bytes ToBytes(const T& value) {
  std::vector<uint8_t> result(sizeof(value));

  T swapped = std::byteswap(value);

  std::memcpy(result.data(), &swapped, sizeof(value));
  return result;
}

template <typename T>
  requires(std::is_trivially_copyable_v<T>)
T Int32FromBytes(const Bytes& bytes) {
  T result;

  ASSERT(bytes.size() == sizeof(result));
  std::memcpy(&result, bytes.data(), sizeof(result));
  return std::byteswap(result);
}

////////////////////////////////////////////////////////////////////////////////

}  // namespace search
