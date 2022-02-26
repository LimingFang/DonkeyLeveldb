#pragma once
#include <cstdint>
#include <cstring>

namespace leveldb {
inline void EncodeFixed32(char* dst, uint32_t data) {
  std::memcpy(dst, &data, sizeof(data));
}
}  // namespace leveldb