#pragma once
#include <string_view>

namespace leveldb {
class Comparator {
 public:
  Comparator() = default;
  virtual ~Comparator() = 0;

  virtual int Compare(std::string_view l, std::string_view r) = 0;

  virtual const char* Name() = 0;
};
}  // namespace leveldb
