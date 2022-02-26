#pragma once

#include <cstdint>

#include "leveldb/slice.h"
#include "leveldb/status.h"
#include "log_format.h"

namespace leveldb {

class WritableFile;
namespace log {
// 需要写一个LogWriter,利用了WritableFile.
class LogWriter {
 public:
  explicit LogWriter(WritableFile* dest);
  LogWriter(const LogWriter&) = delete;

  LogWriter& operator=(const LogWriter&) = delete;
  Status AddRecord(const Slice* slice);

 private:
  Status EmitPhysicalRecord(RecordType type, const char* ptr, int length);

  WritableFile* dest_;

  int block_offset_;
};
};  // namespace log

}  // namespace leveldb