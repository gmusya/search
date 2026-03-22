#include "src/index/lsm_storage.h"

namespace search {

Value BitmapMergeOperator::Merge(const Value& existing, const Value& update) const {
  Bitmap a = Bitmap::Deserialize(existing);
  Bitmap b = Bitmap::Deserialize(update);
  a |= b;
  return a.Serialize();
}

LsmStorage::LsmStorage(std::shared_ptr<IFileSystem> filesystem, Lsm::Parameters params)
    : merge_operator_(std::make_shared<BitmapMergeOperator>()),
      lsm_(std::move(filesystem), params, merge_operator_) {}

void LsmStorage::Add(const Word& word, DocumentId document_id) {
  Key key = WordToKey(word);

  Bitmap bitmap;
  bitmap.Add(document_id);
  Value value = bitmap.Serialize();

  lsm_.Insert(key, value);
}

Bitmap LsmStorage::Get(const Word& word) const {
  Key key = WordToKey(word);
  auto result = lsm_.Get(key);

  if (!result) {
    return Bitmap();
  }

  return Bitmap::Deserialize(*result);
}

Key LsmStorage::WordToKey(const Word& word) {
  return Key(word.begin(), word.end());
}

}  // namespace search
