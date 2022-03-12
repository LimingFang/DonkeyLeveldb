//
// Created by 方里明 on 3/1/22.
//

#include "include/leveldb/comparator.h"

#include <algorithm>

namespace leveldb {
class BytewiseComparator : public Comparator {
 public:
  BytewiseComparator() = default;
  ~BytewiseComparator() override = default;
  int Compare(const Slice& l, const Slice& r) override;
  const char* Name() override;
};

// 字典序
int BytewiseComparator::Compare(const Slice& l, const Slice& r) {
  return l.compare(r);
}
const char* BytewiseComparator::Name() { return "BytewiseComparator"; }

}  // namespace leveldb