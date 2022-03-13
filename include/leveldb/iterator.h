//
// Created by 方里明 on 3/13/22.
//

#ifndef DONKEYLEVELDB_INCLUDE_LEVELDB_ITERATOR_H_
#define DONKEYLEVELDB_INCLUDE_LEVELDB_ITERATOR_H_
#include "leveldb/slice.h"

namespace leveldb {
class Iterator {
 public:
  Iterator();

  Iterator(const Iterator&) = delete;

  Iterator& operator=(const Iterator&) = delete;

  virtual ~Iterator() = 0;

  virtual bool Valid() const = 0;

  virtual void Next() = 0;

  virtual Slice Key() = 0;

  virtual Slice Value() const = 0;

  virtual void SeekToLast() = 0;

  virtual void SeekToFirst() = 0;

  virtual void SeekTo(const Slice& slice) = 0;
};
}  // namespace leveldb
#endif  // DONKEYLEVELDB_INCLUDE_LEVELDB_ITERATOR_H_
