#pragma once

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <map>

#include "src/lsm/file.h"
#include "src/lsm/filesystem.h"
#include "src/lsm/local_file.h"
#include "src/lsm/macro.h"

namespace search {

class LocalFileSystem : public IFileSystem {
 public:
  explicit LocalFileSystem(const std::string& directory) : directory_(directory) {}

  std::string Create() override {
    std::string name = directory_ + "/" + std::to_string(counter_++);
    files_.emplace(name, std::make_unique<FileOpener>(name));
    return name;
  }

  std::shared_ptr<IFile> Open(const std::string& path) override {
    auto it = files_.find(path);
    if (it == files_.end()) {
      THROW_NOT_IMPLEMENTED;
    }

    return it->second->Open();
  }

  void Remove(const std::string& path) override { files_.erase(path); }

 private:
  class FileOpener {
   public:
    explicit FileOpener(const std::string& file_path) : file_path_(file_path) {
      auto res = std::fopen(file_path_.c_str(), "w+");
      ENSURE(res != nullptr);

      std::fclose(res);
    }

    FileOpener(FileOpener&&) = delete;
    FileOpener(const FileOpener&) = delete;

    FileOpener& operator=(const FileOpener&) = delete;
    FileOpener& operator=(const FileOpener&&) = delete;

    std::shared_ptr<IFile> Open() const { return std::make_shared<LocalFile>(file_path_); }

    ~FileOpener() { std::remove(file_path_.c_str()); }

   private:
    std::string file_path_;
  };

  const std::string directory_;
  uint64_t counter_ = 0;
  std::map<std::string, std::unique_ptr<FileOpener>> files_;
};

}  // namespace search
