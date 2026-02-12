#pragma once

#include <memory>
#include <string>

#include "src/file.h"

namespace search {

class IFileSystem {
 public:
  virtual std::string Create() = 0;
  virtual std::shared_ptr<IFile> Open(const std::string& path) = 0;
  virtual void Remove(const std::string& path) = 0;
};

}  // namespace search
