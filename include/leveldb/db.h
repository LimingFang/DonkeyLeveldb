//
// Created by 方里明 on 3/1/22.
//

#ifndef DONKEYLEVELDB_INCLUDE_LEVELDB_DB_H_
#define DONKEYLEVELDB_INCLUDE_LEVELDB_DB_H_
#include <string>
#include <string_view>

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

  virtual Status Put(std::string_view key, std::string_view value) = 0;

  virtual Status Get(std::string_view key, std::string* value) = 0;

  virtual Status Delete(std::string_view key) = 0;
};

}  // namespace leveldb

#endif  // DONKEYLEVELDB_INCLUDE_LEVELDB_DB_H_
