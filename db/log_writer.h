#pragma once

#include "leveldb/slice.h"
#include "leveldb/status.h"
#include "log_format.h"
#include <cstdint>

namespace leveldb {

class WritableFile;    
namespace log {
  // 需要写一个LogWriter,利用了WritableFile.
  class LogWriter {
    public:
      explicit LogWriter(WritableFile* dest);

      LogWriter(WritableFile* dest,uint64_t dest_length);

      LogWriter(const LogWriter&) = delete;

      LogWriter& operator=(const LogWriter&) = delete;

      ~LogWriter();

    Status AddRecord(const Slice* slice);

    private:
      Status EmitPhysicalRecord(RecordType type,const char* ptr,int length);

      WritableFile* dest_;

      int block_offset_;
  };
};    

}