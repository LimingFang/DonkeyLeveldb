//
// Created by 方里明 on 3/1/22.
//

#include "arena.h"

#include <cassert>

namespace leveldb {
const int kBlockSize = 4 << 10;

Arena::Arena() : blocks_({}), ptr_(nullptr), avail_(0) {}

Arena::~Arena() {
  for (auto p : blocks_) {
    delete[] p;
  }
}

char* Arena::Allocate(size_t bytes) {
  assert(bytes > 0);
  if (bytes <= avail_) {
    char* result = ptr_;
    ptr_ += bytes;
    avail_ -= bytes;
    return result;
  }
  return AllocateFallback(bytes);
}

char* Arena::AllocateFallback(size_t bytes) {
  if (bytes >= kBlockSize / 4) {
    return AllocateNewBlock(bytes);
  }
  ptr_ = AllocateNewBlock(kBlockSize);
  avail_ = kBlockSize;
  char* result = ptr_;
  ptr_ += bytes;
  avail_ -= bytes;

  return result;
}

char* Arena::AllocateNewBlock(size_t block_bytes) {
  char* ptr = new char[block_bytes];
  assert(ptr);
  blocks_.push_back(ptr);
  memory_usage_.fetch_add(block_bytes + sizeof(char*),
                          std::memory_order_relaxed);

  return ptr;
}
}  // namespace leveldb