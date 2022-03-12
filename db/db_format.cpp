//
// Created by 方里明 on 3/1/22.
//

#include "db_format.h"
namespace leveldb {

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

}  // namespace leveldb