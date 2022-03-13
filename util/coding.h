#pragma once
#include <cstdint>
#include <cstring>

namespace leveldb {
inline char* EncodeFixed32(char* dst, uint32_t data) {
  std::memcpy(dst, &data, sizeof(data));
  return dst + 4;
}

inline char* EncodeFixed64(char* dst, uint64_t data) {
  std::memcpy(dst, &data, sizeof(data));
  return dst + 8;
}
}  // namespace leveldb