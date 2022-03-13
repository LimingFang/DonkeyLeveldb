//
// Created by 方里明 on 3/1/22.
//

#include "db_format.h"

#include "util/coding.h"
namespace leveldb {

static uint64_t PackSeqnumberAndType(SequenceNumberType seq, ValueType type) {
  return (seq << 8 | type);
}

leveldb::InternalKeyComparator::InternalKeyComparator(Comparator* c)
    : user_key_comparator_(c) {}
int leveldb::InternalKeyComparator::Compare(const leveldb::Slice& l,
                                            const leveldb::Slice& r) {
  int result = user_key_comparator_->Compare(l, r);
  if (result == 0) {
    // 默认小端，seqnum 占据了低7个字节，seqnum 最高字节
    // 存在地址最高处
    // seqnum 更大的反而"更小"
    int idx = l.size() - 1;
    while (true) {
      if (l[idx] > r[idx]) {
        return -1;
      } else if (l[idx] < r[idx]) {
        return 1;
      }
      idx--;
    }
    assert(false);
  }
  return result;
}
const char* leveldb::InternalKeyComparator::Name() {
  return "InternalKeyComparator";
}

LookupKey::LookupKey(const Slice& user_key, SequenceNumberType seq_num) {
  // 如果 s 比较小，就分配在stack上(space_)
  int needed = user_key.size() + 8 + 4 + 1;
  if (needed > sizeof space_) {
    start_ = new char[needed];
  } else {
    start_ = space_;
  }
  // key_length
  kstart_ = EncodeFixed32(start_, user_key.size() + 8);
  // key
  std::memcpy(kstart_, user_key.data(), user_key.size());
  end_ = kstart_ + user_key.size();
  // tag
  end_ = EncodeFixed64(end_, PackSeqnumberAndType(seq_num, kTypeForSeek));
}
Slice LookupKey::memtable_key() { return Slice(start_, end_ - start_); }
Slice LookupKey::internal_key() { return Slice(kstart_, end_ - kstart_); }
Slice LookupKey::user_key() { return Slice(kstart_, end_ - kstart_ - 8); }

}  // namespace leveldb