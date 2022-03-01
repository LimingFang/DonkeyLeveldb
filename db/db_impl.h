//
// Created by 方里明 on 3/1/22.
//

#ifndef DONKEYLEVELDB_DB_DB_IMPL_H_
#define DONKEYLEVELDB_DB_DB_IMPL_H_

#include <string>

#include "leveldb/db.h"
#include "leveldb/slice.h"
#include "leveldb/status.h"

namespace leveldb {

class DBImpl : public DB {
  // 一个存储引擎应当支持的操作：
  // - 打开、关闭DB
  // - 写入数据
  // - 读取数据
  // - 删除数据
 public:
  static Status Open(const std::string& name, DB** dbptr);

  DBImpl() = default;
  DBImpl(const DBImpl&) = delete;
  DBImpl& operator=(const DBImpl&) = delete;
  ~DBImpl() override;

  Status Put(const Slice& key, const Slice& value) override;

  Status Get(const Slice& key, std::string* value) override;

  Status Delete(const Slice& key) override;

 private:
};

}  // namespace leveldb

#endif  // DONKEYLEVELDB_DB_DB_IMPL_H_
