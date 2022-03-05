//
// Created by 方里明 on 3/5/22.
//
#include "util/arena.h"

#include <vector>

#include "util/random.h"

#include "gtest/gtest.h"

namespace leveldb {
// 对于简单的类直接集成测试
TEST(ArenaTest, Empty) { Arena arena; }

TEST(ArenaTest, Simple) {
  Arena arena;
  int N = 1e6;
  int bytes = 0;
  int s;
  char* ptr;
  Random random(335);
  std::vector<std::pair<char*, int>> v;

  // 大部分是小块，但也有少数大块
  for (int i = 0; i < N; i++) {
    if (i % (N / 10) == 0) {
      s = i;
    } else {
      s = random.OneIn(6000) ? random.Uniform(4096)
          : random.OneIn(10) ? random.Uniform(100)
                             : random.Uniform(20);
    }
    if (s == 0) {
      s++;
    }
    ptr = arena.Allocate(s);
    v.emplace_back(ptr, s);
    std::memset(ptr, (s % 128), s);
    bytes += s;
    ASSERT_GE(arena.MemoryUsage(), bytes);
    if (i > N / 10) {
      EXPECT_LE(arena.MemoryUsage(), bytes * 1.05);
    }
  }

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < v[i].second; j++) {
      EXPECT_EQ((v[i].second % 128), v[i].first[j]);
    }
  }
}
}  // namespace leveldb

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
