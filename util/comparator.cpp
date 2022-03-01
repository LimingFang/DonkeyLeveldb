//
// Created by 方里明 on 3/1/22.
//

#include "include/leveldb/comparator.h"

namespace leveldb {
class BytewiseComparator : public Comparator {
 public:
  BytewiseComparator() = default;
  bool Compare(const Slice& l, const Slice& r) override;
  const char* Name() override;
};
}  // namespace leveldb