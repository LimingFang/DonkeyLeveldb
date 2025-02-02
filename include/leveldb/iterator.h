//
// Created by 方里明 on 3/13/22.
//

#pragma once
#include <string_view>
#include "leveldb/status.h"

namespace leveldb {
class Iterator {
 public:
  Iterator();

  Iterator(const Iterator&) = delete;

  Iterator& operator=(const Iterator&) = delete;

  virtual ~Iterator() = 0;

  virtual bool Valid() const = 0;

  virtual void Next() = 0;

  virtual void Prev() = 0;

  virtual std::string_view key() const = 0;

  virtual std::string_view value() const = 0;

  virtual void SeekToLast() = 0;

  virtual void SeekToFirst() = 0;

  virtual void Seek(const std::string_view& slice) = 0;

  virtual Status status() const = 0;
};
}  // namespace leveldb
