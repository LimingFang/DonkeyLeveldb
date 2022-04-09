//
// Created by 方里明 on 3/1/22.
//

#include "db_format.h"

#include "leveldb/slice.h"
#include "util/coding.h"

namespace leveldb {

static uint64_t PackSeqnumberAndType(SequenceNumberType seq, ValueType type) {
  return (seq << 8 | type);
}

InternalKeyComparator::InternalKeyComparator(Comparator* c)
    : user_key_comparator_(c) {}

int InternalKeyComparator::Compare(const Slice& l, const Slice& r) {
  // 优先比较 user_key，如果相等在比较 seq_num
  // 小端，seq_num 在 tag 的低7字节，type 在最高的地址。
  // [type | seq_num]
  // record entry 更靠后 <=> user_key 大 || ((user_key 一致) && seq_num 更小)
  // seq_num 更小 <=> 更旧
  int result =
      user_key_comparator_->Compare(ExtractUserKey(l), ExtractUserKey(r));
  if (result == 0) {
    auto l_tag = DecodeFixed64(l.data() + l.size() - 8);
    auto r_tag = DecodeFixed64(r.data() + r.size() - 8);
    if (l_tag > r_tag) {
      return -1;
    } else {
      return 1;
    }
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