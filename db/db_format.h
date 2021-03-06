//
// Created by 方里明 on 3/1/22.
//

#ifndef DONKEYLEVELDB_DB_DB_FORMAT_H_
#define DONKEYLEVELDB_DB_DB_FORMAT_H_

#include <cstdint>

#include "leveldb/comparator.h"

namespace leveldb {

using SequenceNumberType = uint64_t;
enum ValueType {
  kTypeDeletion = 0x0,
  kTypeValue = 0x1,
};

class LookupKey {};

class InternalKeyComparator : public Comparator {
 public:
  InternalKeyComparator() = default;
  bool Compare(const Slice& l, const Slice& r) override;
  const char* Name() override;
};
}  // namespace leveldb

#endif  // DONKEYLEVELDB_DB_DB_FORMAT_H_
