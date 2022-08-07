//
// Created by 方里明 on 2022/8/7.
//

#include "two_level_iterator.h"

#include "leveldb/options.h"
#include "leveldb/slice.h"

namespace leveldb {
class TwoLevelIterator : public Iterator {
  using BlockFunction = std::unique_ptr<Iterator> (*)(void*, const ReadOptions&,
                                                      const Slice&);

 public:
  TwoLevelIterator(std::unique_ptr<Iterator> index_iter,
                   BlockFunction block_function, void* args,
                   const ReadOptions& options)
      : status_(),
        index_iter_(std::move(index_iter)),
        data_iter_(),
        block_function_(block_function),
        args_(args),
        options_(options),
        data_block_handle_() {}

  ~TwoLevelIterator() override = default;

  void Next() override {
    assert(Valid());
    data_iter_->Next();
    SkipEmptyDataBlockForward();
  }
  void Prev() override {
    assert(Valid());
    data_iter_->Next();
    SkipEmptyDataBlocksBackward();
  }
  void Seek(const Slice& target) override {
    // index_iter.key 性质和 data_iter.key 一致
    index_iter_->Seek(target);
    InitDataBlock();
    if (data_iter_) {
      data_iter_->Seek(target);
    }
    SkipEmptyDataBlockForward();
  }
  void SeekToFirst() override {
    index_iter_->SeekToFirst();
    InitDataBlock();
    if (data_iter_) {
      data_iter_->SeekToFirst();
    }
    // TODO
    SkipEmptyDataBlockForward();
  }
  void SeekToLast() override {
    index_iter_->SeekToLast();
    InitDataBlock();
    if (data_iter_) {
      data_iter_->SeekToLast();
    }
    // TODO
    SkipEmptyDataBlocksBackward();
  }

  bool Valid() const override { return !data_iter_ || data_iter_->Valid(); }
  Slice key() const override {
    assert(Valid());
    return data_iter_->key();
  }
  Slice value() const override {
    assert(Valid());
    return data_iter_->value();
  }
  Status status() const override {
    // TODO
    if (!index_iter_->status().ok()) {
      return index_iter_->status();
    } else if (data_iter_ && !data_iter_->status().ok()) {
      return data_iter_->status();
    } else {
      return status_;
    }
  }

 private:
  // 尽可能保证当前 data_iter 是有效的
  // 如果当前 block 为空或者有问题，就跳一下
  void SkipEmptyDataBlockForward() {
    while (!data_iter_ || !data_iter_->Valid()) {
      if (!index_iter_->Valid()) {
        SetDataIterator(nullptr);
        return;
      }
      index_iter_->Next();
      InitDataBlock();
      if (data_iter_) {
        data_iter_->SeekToFirst();
      }
    }
  }

  void SkipEmptyDataBlocksBackward() {
    while (!data_iter_ || !data_iter_->Valid()) {
      if (!index_iter_->Valid()) {
        SetDataIterator(nullptr);
        return;
      }
      index_iter_->Prev();
      InitDataBlock();
      if (data_iter_) {
        data_iter_->SeekToLast();
      }
    }
  }

  // 根据当前 index_iter value 构造 data_iter
  void InitDataBlock() {
    if (!index_iter_->Valid()) {
      SetDataIterator(nullptr);
    }
    Slice value_handle = index_iter_->value();
    auto iter = block_function_(args_, options_, value_handle);
    data_block_handle_.assign(value_handle.data(), value_handle.size());
    SetDataIterator(std::move(iter));
  }

  // data_iter 为空表示 invalid
  void SetDataIterator(std::unique_ptr<Iterator> data_iter) {
    if (data_iter_ && status_.ok()) {
      status_ = data_iter_->status();
    }
    data_iter_ = std::move(data_iter);
  }

  Status status_;
  std::unique_ptr<Iterator> index_iter_;
  std::unique_ptr<Iterator> data_iter_;
  BlockFunction block_function_;
  void* args_;
  const ReadOptions options_;
  std::string data_block_handle_;
};

std::unique_ptr<Iterator> NewTwoLevelIterator(
    std::unique_ptr<Iterator> index_iter,
    std::unique_ptr<Iterator> (*block_function)(void* args,
                                                const ReadOptions& option,
                                                const Slice& index_value),
    void* args, const ReadOptions& options) {
  return std::make_unique<TwoLevelIterator>(std::move(index_iter),
                                            block_function, args, options);
}
}  // namespace leveldb