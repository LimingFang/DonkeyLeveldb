//
// Created by 方里明 on 3/1/22.
//

#ifndef DONKEYLEVELDB_DB_MEMTABLE_H_
#define DONKEYLEVELDB_DB_MEMTABLE_H_

#include "db/db_format.h"
#include "db/skiplist.h"
#include <string>

#include "leveldb/iterator.h"
#include "leveldb/status.h"

#include "util/arena.h"

namespace leveldb {
class MemTableIterator;
class MemTable {
  friend class MemTableIterator;

 public:
  MemTable(const InternalKeyComparator& comparator);
  MemTable(const MemTable&) = delete;
  MemTable& operator=(const MemTable&) = delete;

  // format: key_len + key + seq + type + value len + value.
  void Add(SequenceNumberType seq, ValueType type, const Slice& key,
           const Slice& value);

  bool Get(const LookupKey& key, std::string* value, Status* st);

  Iterator* NewIterator();

 private:
  struct KeyComparator {
    // wrapper for InternalKey
    const InternalKeyComparator comparator;
    explicit KeyComparator(InternalKeyComparator c) : comparator(c) {}
    bool operator()(const char* a, const char* b);
  };
  KeyComparator comparator_;
  int refs_;
  using Table = SkipList<KeyComparator>;
  Table table_;
  Arena arena_;
};

}  // namespace leveldb

#endif  // DONKEYLEVELDB_DB_MEMTABLE_H_
