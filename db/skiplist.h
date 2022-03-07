//
// Created by 方里明 on 3/1/22.
//

#ifndef DONKEYLEVELDB_DB_SKIPLIST_H_
#define DONKEYLEVELDB_DB_SKIPLIST_H_

#include "leveldb/slice.h"
#include "util/arena.h"

namespace leveldb {
template <typename Key, class Comparator>
class SkipList {
 private:
  struct Node;

 public:
  // 一些规定:高度值[1,kMaxHeight]
  SkipList(Comparator* comparator, Arena* arena);

  void Insert(const Key& key);

  bool Contains(const Key& key);

 private:
  int GetRandomHeight();

  Node* FindGreaterOrEqual(const Key& key, Node** prev);

  Node* NewNode(int level);

  bool KeyIsAfterNode(const Key& key, Node* node);

  Arena* arena_;
  Comparator* comparator_;
  int max_height_;
  Node* header_;
  static const int kMaxHeight = 12;
};

template <typename Key, class Comparator>
struct SkipList<Key, Comparator>::Node {
  explicit Node(const Key& key) : key(key){};
  void SetNext(int level, Node* node) { next[level - 1] = node; }
  Node* Next(int level) { return next[level - 1]; }

  Key key;

 private:
  Node* next[0];
};

template <typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::NewNode(
    int level) {
  char* mem = arena_->Allocate(sizeof(Node) + sizeof(Node*) * (level - 1));
  return new (mem) Node;
}

template <typename Key, class Comparator>
SkipList<Key, Comparator>::SkipList(Comparator* comparator, Arena* arena)
    : comparator_(comparator),
      arena_(arena),
      max_height_(1),
      header_(NewNode(kMaxHeight)) {
  for (int i = 0; i < kMaxHeight; i++) {
    header_[i] = nullptr;
  }
}

template <typename Key, class Comparator>
bool SkipList<Key, Comparator>::Contains(const Key& key) {
  Node* node = FindGreaterOrEqual(key, nullptr);
  return node && comparator_(node->key, key) == 0;
}

template <typename Key, class Comparator>
bool SkipList<Key, Comparator>::KeyIsAfterNode(const Key& key, Node* node) {
  return comparator_(key, node->key) > 0;
}

template <typename Key, class Comparator>
void SkipList<Key, Comparator>::Insert(const Key& key) {
  Node prev[kMaxHeight];
  Node* next = FindGreaterOrEqual(key, &prev);
  if (next && comparator_(next->key, key) == 0) {
    return;
  }
  int h = GetRandomHeight();
  Node* new_node = NewNode(h);
  if (max_height_ < h) {
    for (int i = h; i > max_height_; i--) {
      prev[i - 1] = header_;
    }
    max_height_ = h;
  }
  for (int i = 1; i <= h; i++) {
    new_node->SetNext(i, prev[i - 1].Next(i));
    prev[i - 1].SetNext(i, new_node);
  }
}

template <typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node*
SkipList<Key, Comparator>::FindGreaterOrEqual(const Key& key, Node** prev) {
  Node* n = header_;
  int lev = max_height_;
  // 在当前层查找next，如果next更大，往下一层（if）
  // 如果next小，则往前走一步
  while (true) {
    Node* next = n->Next(lev);
    if (KeyIsAfterNode(key, next)) {
      n = next;
    } else {
      if (prev) {
        prev[lev - 1] = n;
      }
      lev--;
      if (lev == 0) {
        return next;
      }
    }
  }
}

}  // namespace leveldb

#endif  // DONKEYLEVELDB_DB_SKIPLIST_H_
