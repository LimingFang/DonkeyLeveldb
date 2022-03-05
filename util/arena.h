//
// Created by 方里明 on 3/1/22.
//

#ifndef DONKEYLEVELDB_UTIL_ARENA_H_
#define DONKEYLEVELDB_UTIL_ARENA_H_
#include <cstddef>
#include <vector>

namespace leveldb {
class Arena {
 public:
  Arena();
  ~Arena();

  Arena(const Arena&) = delete;
  Arena& operator=(const Arena&) = delete;

  char* Allocate(size_t bytes);

 private:
  char* AllocateFallback(size_t bytes);

  char* AllocateNewBlock(size_t block_bytes);

  std::vector<char*> blocks_;
  char* ptr_;
  int avail_;
  std::atomic<size_t> memory_usage_;
};
}  // namespace leveldb

#endif  // DONKEYLEVELDB_UTIL_ARENA_H_
