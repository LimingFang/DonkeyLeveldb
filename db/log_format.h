#pragma once
#include <cstdint>
namespace leveldb {

namespace log {
// 定义了字段类型、以及record类型
enum RecordType : uint8_t {
  kZeroType = 0,

  kFullType = 2,

  kFirstType = 4,

  kMidType = 6,

  kLastType = 8,
};

static const int kBlockSize = 32 << 10;

static const int kHeaderSize = 4 + 2 + 1;
};  // namespace log

}  // namespace leveldb