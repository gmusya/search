#pragma once

#include <iostream>
#include <memory>
#include <random>

#include "src/file.h"
#include "src/types.h"

namespace search {

namespace internal {
class Hasher {
 public:
  Hasher(uint64_t seed) {
    std::mt19937_64 value(seed);

    to_xor_with_ = value();
  }

  uint64_t Hash(uint64_t x) const {
    x ^= to_xor_with_;
    x = (x ^ (x >> 30)) * static_cast<uint64_t>(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * static_cast<uint64_t>(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
  }

  uint64_t Hash(const Bytes& bytes) const {
    uint64_t result = 0;
    for (uint64_t i = 0; i * 8 < bytes.size(); ++i) {
      uint64_t current;
      std::memcpy(&current, bytes.data() + i, sizeof(current));

      result = Hash(result) + Hash(current);
    }

    return result;
  }

 private:
  uint64_t to_xor_with_;
};
}  // namespace internal

struct BloomFilterParameters {
  uint32_t bytes;
  uint32_t functions;
};

class BloomFilterReader {
 public:
  explicit BloomFilterReader(std::shared_ptr<const IFile> file) : file_(file) { Load(); }

  bool Contains(const Key& key) const {
    for (uint32_t i = 0; i < params_.functions; ++i) {
      uint64_t bit = hashers_[i].Hash(key) % (8 * mask_.size());

      if ((mask_[bit >> 3] & (1 << (bit & 7))) == 0) {
        return false;
      }
    }

    return true;
  }

 private:
  void Load() {
    uint32_t sz;
    file_->Read(0, sizeof(params_), &params_);
    file_->Read(sizeof(params_), sizeof(sz), &sz);

    mask_.resize(sz);
    file_->Read(sizeof(params_) + sizeof(sz), mask_.size(), mask_.data());

    for (uint32_t i = 0; i < params_.functions; ++i) {
      hashers_.emplace_back(i);
    }
  }

  std::shared_ptr<const IFile> file_;

  BloomFilterParameters params_;
  std::vector<uint8_t> mask_;

  std::vector<internal::Hasher> hashers_;
};

class BloomFilterWriter {
 public:
  explicit BloomFilterWriter(std::shared_ptr<IFile> file, BloomFilterParameters params)
      : file_(file), params_(params), mask_(params.bytes) {
    for (uint32_t i = 0; i < params.functions; ++i) {
      hashers_.emplace_back(i);
    }
  }

  void Add(const Key& entry) {
    for (uint32_t i = 0; i < params_.functions; ++i) {
      uint64_t bit = hashers_[i].Hash(entry) % (8 * mask_.size());

      mask_[bit >> 3] |= (1 << (bit & 7));
    }
  }

  void Finish() && {
    file_->Resize(sizeof(params_) + sizeof(uint32_t) + mask_.size());

    uint32_t sz = mask_.size();
    file_->Write(0, &params_, sizeof(params_));
    file_->Write(sizeof(params_), &sz, sizeof(sz));

    file_->Write(sizeof(params_) + sizeof(sz), mask_.data(), mask_.size());
  }

 private:
  std::shared_ptr<IFile> file_;

  BloomFilterParameters params_;
  std::vector<uint8_t> mask_;

  std::vector<internal::Hasher> hashers_;
};

}  // namespace search
