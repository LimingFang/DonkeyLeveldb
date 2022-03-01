//
// Created by 方里明 on 3/1/22.
//

#ifndef DONKEYLEVELDB_DB_MEMTABLE_H_
#define DONKEYLEVELDB_DB_MEMTABLE_H_

#include <string>

#include "db/db_format.h"
#include "db/skiplist.h"
#include "leveldb/status.h"

namespace leveldb {

class MemTable {
 public:
  MemTable();
  MemTable(const MemTable&) = delete;
  MemTable& operator=(const MemTable&) = delete;

  void Add(SequenceNumberType seq, ValueType type, const Slice& key,
           const Slice& value);

  bool Get(const LookupKey& key, std::string* value, Status* st);

 private:
  struct KeyComparator {
    // wrap for InternalKey
    const InternalKeyComparator comparator;
    explicit KeyComparator(InternalKeyComparator c) : comparator(c) {}
    bool operator()(const char* a, const char* b);
  };
  using Table = SkipList<const char*, KeyComparator>;
  Table* table_;
};

}  // namespace leveldb

#endif  // DONKEYLEVELDB_DB_MEMTABLE_H_
