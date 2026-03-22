#pragma once

#include <cstring>
#include <vector>

#include "src/lsm/assert.h"
#include "src/lsm/file.h"

namespace search {

class MemoryFile : public IFile {
 public:
  void Read(uint64_t offset, uint64_t bytes, void* output) const override {
    ASSERT(offset + bytes <= storage_.size());
    std::memcpy(output, storage_.data() + offset, bytes);
  }

  void Write(uint64_t offset, const void* data, uint64_t size) override {
    if (storage_.size() < offset + size) {
      storage_.resize(offset + size);
    }
    std::memcpy(storage_.data() + offset, data, size);
  }

  void Resize(uint64_t size) override { storage_.resize(size); }

  uint64_t Size() const override { return storage_.size(); }

 private:
  std::vector<uint8_t> storage_;
};

}  // namespace search
