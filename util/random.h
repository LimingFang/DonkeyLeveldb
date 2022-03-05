//
// Created by 方里明 on 3/5/22.
//

#ifndef DONKEYLEVELDB_UTIL_RANDOM_H_
#define DONKEYLEVELDB_UTIL_RANDOM_H_

#include <cstdint>
#include <random>
namespace leveldb {
// 随机数生成需要：seed，engine，transformer
// Random 负责提供一些工具函数，例如OneIn(n)
class Random {
 public:
  explicit Random(int seed) : gen_(seed), dist_(0, UINT32_MAX) {}

  uint32_t Next() { return dist_(gen_); }

  bool OneIn(int n) {
    auto s = Next();
    return (s % n) == 0;
  }

  // return [0,n-1]
  uint32_t Uniform(int n) { return Next() % n; }

 private:
  std::mt19937 gen_;
  std::uniform_int_distribution<uint32_t> dist_;
};
}  // namespace leveldb

#endif  // DONKEYLEVELDB_UTIL_RANDOM_H_
