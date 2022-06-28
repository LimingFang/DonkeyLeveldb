//
// Created by 方里明 on 3/1/22.
//

#ifndef DONKEYLEVELDB_DB_SKIPLIST_H_
#define DONKEYLEVELDB_DB_SKIPLIST_H_

#include "leveldb/slice.h"

#include "util/arena.h"
#include "util/random.h"

namespace leveldb {
template <class Comparator>
class SkipList {
 private:
  struct Node;

 public:
  SkipList() = default;
  // Node->next[i]表示Node高度为(i+1)的 next 节点
  // 一些规定:高度值[1,kMaxHeight]
  SkipList(Comparator comparator, Arena* arena);

  // No duplicate key is allowed.
  void Insert(const char* key);

  bool Contains(const char* key);

  class Iterator {
   public:
    explicit Iterator(SkipList* skip_list)
        : current_node_(skip_list->header_), list_(skip_list){};

    bool Valid() const;

    void Next();

    Slice Value() const;

    void SeekToLast();

    void SeekToFirst();

    // 定位到第一个entry(entry.key >= key).
    void SeekTo(const char* key);

   private:
    Node* current_node_;
    SkipList* list_;
  };

 private:
  // For Generating new node.
  int GetRandomHeight();

  // comparator(a,b) <= 0 means nodeA isn't before nodeB.
  // return NodeX which comparator(key,X) <= 0.
  Node* FindGreaterOrEqual(const char* key, Node** prev);

  // .
  Node* NewNode(const char* key, int height);

  // 返回最后一个节点，若为空则返回 header_
  // For what?
  Node* FindLast() const;

  // Return true when comparator(key,node) > 0.
  // For example, comparator = ">",即 skiplist 升序排列{1,3,5}.
  // Keyisafternode(3,5) = false.
  // Keyisafternode(4,3) = true.
  // keyisafternode(6,nullptr) = false.
  bool KeyIsAfterNode(const char* key, Node* node);

 private:
  Arena* arena_;
  Comparator comparator_;
  int max_height_;
  Node* header_;
  static const int kMaxHeight = 12;
};

template <class Comparator>
bool SkipList<Comparator>::Iterator::Valid() const {
  return current_node_ != nullptr;
}

template <class Comparator>
void SkipList<Comparator>::Iterator::Next() {
  assert(current_node_ != nullptr);
  current_node_ = current_node_->Next(0);
}

template <class Comparator>
Slice SkipList<Comparator>::Iterator::Value() const {
  assert(current_node_ != nullptr);
  return Slice(current_node_->key);
}

template <class Comparator>
void SkipList<Comparator>::Iterator::SeekToLast() {
  current_node_ = list_->FindLast();
  if (current_node_ == list_->header_) {
    current_node_ = nullptr;
  }
}

template <class Comparator>
void SkipList<Comparator>::Iterator::SeekToFirst() {
  current_node_ = list_->header_->Next(0);
}

template <class Comparator>
void SkipList<Comparator>::Iterator::SeekTo(const char* key) {
  current_node_ = list_->FindGreaterOrEqual(key, nullptr);
}

template <class Comparator>
struct SkipList<Comparator>::Node {
  explicit Node(const char* key) : key(key){};
  void SetNext(int level, Node* node) { next[level] = node; }
  Node* Next(int level) { return next[level]; }

  const char* key;

 private:
  Node* next[0];
};

template <class Comparator>
typename SkipList<Comparator>::Node* SkipList<Comparator>::NewNode(
    const char* key, int height) {
  char* mem = arena_->Allocate(sizeof(Node) + sizeof(Node*) * height);
  return new (mem) Node(key);
}

template <class Comparator>
SkipList<Comparator>::SkipList(Comparator comparator, Arena* arena)
    : comparator_(comparator),
      arena_(arena),
      max_height_(1),
      header_(NewNode(0, kMaxHeight)) {
  for (int i = 0; i < kMaxHeight; i++) {
    header_->SetNext(i, nullptr);
  }
}

template <class Comparator>
bool SkipList<Comparator>::Contains(const char* key) {
  Node* node = FindGreaterOrEqual(key, nullptr);
  return node && comparator_(node->key, key) == 0;
}

template <class Comparator>
bool SkipList<Comparator>::KeyIsAfterNode(const char* key, Node* node) {
  return !node || comparator_(key, node->key) > 0;
}

template <class Comparator>
void SkipList<Comparator>::Insert(const char* key) {
  Node* prev[kMaxHeight];
  // next >= key.
  Node* next = FindGreaterOrEqual(key, prev);

  // No duplicate key is allowed.
  assert(!next || comparator_(next->key, key) == 0);

  int h = GetRandomHeight();
  Node* new_node = NewNode(key, h);
  // 如果新节点超高，则超出的部分前驱节点是header（此时是nullptr）
  if (max_height_ < h) {
    for (int i = h; i >= max_height_; i--) {
      prev[i - 1] = header_;
    }
    max_height_ = h;
  }
  // 每个前驱节点新的后驱节点需要更新
  for (int i = 0; i < h; i++) {
    new_node->SetNext(i, prev[i]->Next(i));
    prev[i]->SetNext(i, new_node);
  }
}

template <class Comparator>
int SkipList<Comparator>::GetRandomHeight() {
  auto r = Random(10);
  int height = 1;
  while (r.OneIn(4)) {
    height++;
    if (height == kMaxHeight) {
      break;
    }
  }
  return height;
}

template <class Comparator>
typename SkipList<Comparator>::Node* SkipList<Comparator>::FindLast() const {
  if (header_->Next(0) == nullptr) {
    return header_;
  }
  Node* n = header_;
  int h = max_height_ - 1;
  while (true) {
    Node* next = n->Next(h);
    if (next) {
      continue;
    }
    if (h == 0) {
      break;
    }
    h--;
  }
  return n;
}

template <class Comparator>
typename SkipList<Comparator>::Node* SkipList<Comparator>::FindGreaterOrEqual(
    const char* key, Node** prev) {
  Node* n = header_;
  int lev = max_height_ - 1;
  while (true) {
    Node* next = n->Next(lev);
    if (KeyIsAfterNode(key, next)) {
      n = next;
    } else {
      if (prev) {
        prev[lev] = n;
      }
      lev--;
      if (lev == -1) {
        return next;
      }
    }
  }
}

}  // namespace leveldb

#endif  // DONKEYLEVELDB_DB_SKIPLIST_H_
