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

inline uint64_t DecodeFixed64(const char* src) {
  uint64_t data = 0;
  std::memcpy(&data, src, 8);
  return data;
}

int VariantLength(uint64_t num) {
  int len = 1;
  while (num > 128) {
    len++;
    num >>= 7;
  }
  return len;
}

char* EncodeVariant(char* c, uint64_t num) {
  auto* ptr = reinterpret_cast<uint8_t*>(c);
  while (num > 128) {
    *ptr = (num & 0x7f);
    ptr++;
    num >>= 7;
  }
  *ptr = num;
  return reinterpret_cast<char*>(ptr + 1);
}

const char* GetVariant32Ptr(const char* p, const char* limit, uint32_t* length);

}  // namespace leveldb
