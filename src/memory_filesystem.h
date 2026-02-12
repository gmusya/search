#pragma once

#include <map>

#include "src/filesystem.h"
#include "src/macro.h"
#include "src/memory_file.h"

namespace search {

class MemoryFileSystem : public IFileSystem {
 public:
  std::string Create() override {
    std::string name = "memory://" + std::to_string(counter_++);

    auto new_file = std::make_shared<MemoryFile>();

    files_[name] = new_file;

    return name;
  }

  std::shared_ptr<IFile> Open(const std::string& path) override {
    auto it = files_.find(path);
    if (it == files_.end()) {
      THROW_NOT_IMPLEMENTED;
    }

    return it->second;
  }

  void Remove(const std::string& path) override { files_.erase(path); }

 private:
  uint64_t counter_ = 0;
  std::map<std::string, std::shared_ptr<IFile>> files_;
};

}  // namespace search
