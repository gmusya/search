#pragma once

#include <cstring>
#include <filesystem>
#include <fstream>
#include <ios>

#include "src/assert.h"
#include "src/file.h"

namespace search {

class LocalFile : public IFile {
 public:
  LocalFile(const std::string& file_path)
      : file_path_(file_path), file_(file_path, std::ios_base::in | std::ios_base::out | std::ios_base::binary) {}

  void Read(uint64_t offset, uint64_t bytes, void* output) const override {
    file_.seekp(offset);
    file_.read(reinterpret_cast<char*>(output), bytes);
  }

  void Write(uint64_t offset, const void* data, uint64_t size) override {
    file_.seekp(offset);
    file_.write(reinterpret_cast<const char*>(data), size);
    file_.flush();
  }

  void Resize(uint64_t size) override { std::filesystem::resize_file(file_path_, size); }

  uint64_t Size() const override { return std::filesystem::file_size(file_path_); }

 private:
  const std::string file_path_;

  mutable std::fstream file_;
};

}  // namespace search
