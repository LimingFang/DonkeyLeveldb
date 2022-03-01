//
// Created by 方里明 on 3/1/22.
//

#ifndef DONKEYLEVELDB_DB_SKIPLIST_H_
#define DONKEYLEVELDB_DB_SKIPLIST_H_

#include "leveldb/slice.h"
#include "util/arena.h"

namespace leveldb {
template <typename Key, class Comparator>
class SkipList {
 public:
  SkipList(Comparator* comparator, Arena* arena);

  void Insert(const Key& key);

  bool Contains(const Key& key);

 private:
  Arena* arena_;
};
}  // namespace leveldb

#endif  // DONKEYLEVELDB_DB_SKIPLIST_H_
