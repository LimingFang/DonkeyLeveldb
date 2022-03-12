//
// Created by 方里明 on 3/1/22.
//

#ifndef DONKEYLEVELDB_INCLUDE_LEVELDB_COMPARATOR_H_
#define DONKEYLEVELDB_INCLUDE_LEVELDB_COMPARATOR_H_
#include "leveldb/slice.h"

namespace leveldb {
class Comparator {
 public:
  Comparator() = default;
  virtual ~Comparator() = 0;

  virtual int Compare(const Slice& l, const Slice& r) = 0;

  virtual const char* Name() = 0;
};
}  // namespace leveldb

#endif  // DONKEYLEVELDB_INCLUDE_LEVELDB_COMPARATOR_H_
