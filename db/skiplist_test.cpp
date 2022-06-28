//
// Created by 方里明 on 3/18/22.
//
#include "db/skiplist.h"

#include <algorithm>
#include <cstring>
#include <random>
#include <set>
#include <vector>

#include "util/arena.h"

#include "gtest/gtest.h"

namespace leveldb {
// 首先测试 Insert 和 Contain，然后测试迭代器
// Key:const char*
// Comparator:自定义
class TestSkipList : public ::testing::Test {
 protected:
  struct Comparator {
    bool operator()(const char* a, const char* b) {
      // 字典序
      int min_len = std::min(strlen(a), strlen(b));
      int common = std::memcmp(a, b, min_len);
      if (common == 0) {
        return strlen(a) > strlen(b);
      }
      return common;
    }
  };

  SkipList<Comparator> skip_list_;
  Arena* arena_;
  Arena* arena_for_test_;
  Comparator comparator_;
  const char* str =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  int len;

  void SetUp() override {
    arena_ = new Arena();
    arena_for_test_ = new Arena();
    comparator_ = Comparator{};
    skip_list_ = SkipList<Comparator>(comparator_, arena_);
    len = strlen(str);
  }
  void TearDown() override {
    delete arena_;
    delete arena_for_test_;
  }

  const char* random_char() {
    char* s = static_cast<char*>(arena_for_test_->Allocate(len));
    std::memcpy(s, str, len);
    std::random_device rd;
    std::mt19937 generator(rd());
    std::shuffle(s, s + len, generator);
    return s;
  }
};

TEST_F(TestSkipList, ApiTest) {
  // 随机插入1000个数据，查询
  const int n = 1000;
  std::set<const char*> s;
  for (int i = 0; i < n; i++) {
    s.insert(random_char());
  }
  for (auto elem : s) {
    skip_list_.Insert(elem);
  }
  for (auto elem : s) {
    EXPECT_TRUE(skip_list_.Contains(elem));
  }
}

// 生成一个一定容量的 skiplist，测试各个迭代器的功能。
TEST_F(TestSkipList, IteratorTest1) {}
}  // namespace leveldb

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
