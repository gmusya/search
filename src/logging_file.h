#pragma once

#include <cstring>
#include <memory>

#include "src/file.h"

namespace search {

class LoggingFile : public IFile {
 public:
  explicit LoggingFile(std::shared_ptr<IFile> file) : file_(file) {}

  void Read(uint64_t offset, uint64_t bytes, void* output) const override {
    bytes_read_ += bytes;
    file_->Read(offset, bytes, output);
  }

  void Write(uint64_t offset, const void* data, uint64_t size) override {
    bytes_written_ += size;
    file_->Write(offset, data, size);
  }

  void Resize(uint64_t size) override { file_->Resize(size); }

  uint64_t Size() const override { return file_->Size(); }

  uint64_t BytesRead() const { return bytes_read_; }

  uint64_t BytesWritten() const { return bytes_written_; }

 private:
  std::shared_ptr<IFile> file_;

  mutable uint64_t bytes_read_ = 0;
  mutable uint64_t bytes_written_ = 0;
};

}  // namespace search
