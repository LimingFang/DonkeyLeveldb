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

// TODO
const ValueType kTypeForSeek = kTypeValue;

inline Slice ExtractUserKey(const Slice& internal_key) {
  return Slice(internal_key.data(),
               internal_key.size() - sizeof(SequenceNumberType));
}

// 给定 user_key 和 seq_num，去 memtable 中查询
// 对应的 record。
class LookupKey {
 public:
  LookupKey(const Slice& user_key, SequenceNumberType seq_num);

  Slice memtable_key() const;

  Slice internal_key() const;

  Slice user_key() const;

 private:
  // key_length 第一个字节
  char* start_;
  // key 第一个字节
  char* kstart_;
  // tag 最后一个字节
  char* end_;

  char space_[200];
};

class InternalKeyComparator : public Comparator {
 public:
  explicit InternalKeyComparator(Comparator* c);
  ~InternalKeyComparator() override = default;
  int Compare(const Slice& l, const Slice& r) override;
  const char* Name() override;
  Comparator* user_comparator() const { return user_key_comparator_; }

 private:
  Comparator* user_key_comparator_;
};
}  // namespace leveldb

#endif  // DONKEYLEVELDB_DB_DB_FORMAT_H_
