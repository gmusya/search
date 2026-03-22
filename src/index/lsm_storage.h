#pragma once

#include <memory>

#include "src/index/bitmap.h"
#include "src/index/index.h"
#include "src/lsm/lsm.h"
#include "src/lsm/merge_operator.h"

namespace search {

class BitmapMergeOperator : public IMergeOperator {
 public:
  Value Merge(const Value& existing, const Value& update) const override;
};

class LsmStorage : public IStorage {
 public:
  explicit LsmStorage(std::shared_ptr<IFileSystem> filesystem,
                      Lsm::Parameters params = Lsm::Parameters::Default());

  void Add(const Word& word, DocumentId document_id) override;
  Bitmap Get(const Word& word) const override;
  Bitmap GetRange(const Word& min, const Word& max) const override;

 private:
  static Key WordToKey(const Word& word);

  std::shared_ptr<BitmapMergeOperator> merge_operator_;
  mutable Lsm lsm_;
};

}  // namespace search
