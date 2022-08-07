//
// Created by 方里明 on 2022/8/7.
//

#include "table/merger.h"

#include "leveldb/comparator.h"
#include "leveldb/iterator.h"
#include "leveldb/slice.h"
#include "leveldb/status.h"

namespace leveldb {
class MergingIterator : public Iterator {
 public:
  MergingIterator(Comparator* comparator, std::vector<Iterator*> children)
      : direction_(kForward),
        comparator_(comparator),
        children_(std::move(children)),
        current_(nullptr) {}
  MergingIterator() = default;

  void Next() override {
    assert(Valid());
    if (direction_ != kForward) {
      // 调整除 current 外的迭代器，保证指向的数据>=current.data
      for (auto iter : children_) {
        if (iter == current_) {
          continue;
        }
        while (iter->Valid() &&
               !comparator_->Compare(iter->key(), current_->key())) {
          iter->Next();
        }
        // 此时有可能出现 iter 还没有迭代到位但已经失效，但这不会立即影响到整个
        // `MergingIterator` 的工作，只有我们想要的数据在它上面才会 invalid
      }
      direction_ = kForward;
    }
    current_->Next();
    FindSmallest();
  }
  void Prev() override {
    assert(Valid());
    if (direction_ != kBackward) {
      for (auto iter : children_) {
        if (iter == current_) {
          continue;
        }
        while (iter->Valid() &&
               !comparator_->Compare(current_->key(), iter->key())) {
          iter->Prev();
        }
      }
      direction_ = kBackward;
    }
    current_->Prev();
    FindLargest();
  }
  void Seek(const Slice& slice) override {}
  void SeekToFirst() override {}
  void SeekToLast() override {}

  Slice key() const override {
    assert(Valid());
    return current_->key();
  }
  Slice value() const override {
    assert(Valid());
    return current_->value();
  }
  Status status() const override {
    // 刚构建时是 invalid
    // Seek 后正常来说是 valid，也有可能 invalid
    // 从 Valid 开始运行，比如 Next，可能导致 invalid
    //
    // `MergingIterator` 底层是 `TwoLevelIterator`,其持有
    // index_block_iterator+data_block_iterator，后者在
    // 迭代完 sst 文件后就变成 invalid，但 status 是 ok 的。
    // 因此可以认为 invalid 不一定 !Status::ok().
    // 因此问题就变成了：当底层有错误时，上层是否立即反应。
    // leveldb 选择是返回这个错误，但是此时可能还可以继续迭代一些数据。
    for (auto iter : children_) {
      if (!iter->status().ok()) {
        return iter->status();
      }
    }
    return Status::OK();
  }
  bool Valid() const override {
    // `current_` 非空意味着还能迭代
    return current_ != nullptr;
  }

 private:
  enum Direction {
    kForward,
    kBackward,
  };

  // `Next` 内部调用，找出下次迭代数据所在 iterator.
  // 有可能遇到所有 iterator 都失效的情况
  void FindSmallest() {
    Iterator* small = nullptr;
    for (auto iter : children_) {
      if (iter->Valid()) {
        if (!small) {
          small = iter;
        } else if (comparator_->Compare(iter->key(), small->key())) {
          small = iter;
        }
      }
    }
    current_ = small;
  }
  void FindLargest() {
    Iterator* large = nullptr;
    for (auto iter : children_) {
      if (iter->Valid()) {
        if (!large) {
          large = iter;
        } else if (comparator_->Compare(iter->key(), large->key())) {
          large = iter;
        }
      }
    }
    current_ = large;
  }

  Direction direction_;
  Comparator* comparator_;
  std::vector<Iterator*> children_;
  Iterator* current_;
};

Iterator* NewMergingIterator(Comparator* comparator,
                             std::vector<Iterator*> children) {
  return new MergingIterator(comparator, std::move(children));
}
}  // namespace leveldb