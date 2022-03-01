//
// Created by 方里明 on 3/1/22.
//

#ifndef DONKEYLEVELDB_UTIL_ARENA_H_
#define DONKEYLEVELDB_UTIL_ARENA_H_
#include <cstddef>

namespace leveldb {
class Arena {
 public:
  Arena();
  ~Arena();

  Arena(const Arena&) = delete;
  Arena& operator=(const Arena&) = delete;

  char* Allocate(size_t bytes);
};
}  // namespace leveldb

#endif  // DONKEYLEVELDB_UTIL_ARENA_H_
