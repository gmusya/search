#pragma once

#include <cstdint>

namespace search {

class IFile {
 public:
  virtual void Read(uint64_t offset, uint64_t bytes, void* output) const = 0;
  virtual void Write(uint64_t offset, const void* data, uint64_t size) = 0;
  virtual void Resize(uint64_t size) = 0;
  virtual uint64_t Size() const = 0;

  virtual ~IFile() = default;
};

}  // namespace search
