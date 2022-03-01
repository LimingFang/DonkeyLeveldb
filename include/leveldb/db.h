//
// Created by 方里明 on 3/1/22.
//

#ifndef DONKEYLEVELDB_INCLUDE_LEVELDB_DB_H_
#define DONKEYLEVELDB_INCLUDE_LEVELDB_DB_H_
#include <string>

#include "leveldb/slice.h"
#include "leveldb/status.h"

namespace leveldb {

class DB {
  // 一个存储引擎应当支持的操作：
  // - 打开、关闭DB
  // - 写入数据
  // - 读取数据
  // - 删除数据
 public:
  static Status Open(const std::string& name, DB** dbptr);

  DB() = default;
  DB(const DB&) = delete;
  DB& operator=(const DB&) = delete;
  virtual ~DB();

  virtual Status Put(const Slice& key, const Slice& value) = 0;

  virtual Status Get(const Slice& key, std::string* value) = 0;

  virtual Status Delete(const Slice& key) = 0;
};

}  // namespace leveldb

#endif  // DONKEYLEVELDB_INCLUDE_LEVELDB_DB_H_
