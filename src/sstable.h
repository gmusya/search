#pragma once

#include <memory>
#include <optional>

#include "src/file.h"
#include "src/types.h"

namespace search {

class SSTableReader {
 public:
  explicit SSTableReader(std::shared_ptr<const IFile> file) : file_(file) { Load(); }

  enum class Status { kNotFound, kDeleted, kFound };

  Status Get(const Key& key, Value* result) const;

  std::vector<std::pair<InternalKey, Value>> ReadRange(std::optional<Key> min, std::optional<Key> max) const;

 private:
  uint32_t SearchForKey(Key key) const;

  void Load();

  uint32_t OffsetAt(uint32_t index) const;

  InternalKey ReadKey(uint32_t offset) const;

  std::pair<InternalKey, Value> ReadEntry(uint32_t offset) const;

  std::shared_ptr<const IFile> file_;

  uint64_t file_size_ = 0;
  uint32_t entries_count_ = 0;
};

class SSTableWriter {
 public:
  explicit SSTableWriter(std::shared_ptr<IFile> file) : file_(file) {}

  void Write(const std::vector<std::pair<InternalKey, Value>>& entries) &&;

 private:
  std::shared_ptr<IFile> file_;
};

}  // namespace search
