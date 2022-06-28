#include "log_writer.h"

#include "leveldb/env.h"
#include "leveldb/status.h"

#include "util/coding.h"
#include "util/crc32c.h"

#include "log_format.h"

namespace leveldb {
namespace log {

LogWriter::LogWriter(WritableFile* dest) : dest_(dest), block_offset_(0) {}

// 将slice写入当前的file中，内部会处理好block和record的关系
Status LogWriter::AddRecord(const Slice* slice) {
  Status st;
  int leftover = slice->size();
  bool first = true;
  const char* ptr = slice->data();

  do {
    int block_space = kBlockSize - block_offset_;
    if (block_space < 7) {
      if (block_space > 0) {
        st = dest_->Append(Slice("\x00\x00\x00\x00\x00\x00\x00", block_space));
      }
      block_offset_ = 0;
      block_space = kBlockSize;
    }

    int fragment_length = (leftover > (block_space - kHeaderSize))
                              ? (block_space - kHeaderSize)
                              : leftover;

    bool end = fragment_length == leftover;
    RecordType type;
    if (first & end) {
      type = kFullType;
    } else if (first) {
      type = kFirstType;
    } else if (end) {
      type = kLastType;
    } else {
      type = kMidType;
    }

    st = EmitPhysicalRecord(type, ptr, fragment_length);
    leftover -= fragment_length;
    ptr += fragment_length;
    first = false;
  } while (st.ok() && leftover > 0);

  return st;
}

// 将给定的content按照log format写入file
Status LogWriter::EmitPhysicalRecord(RecordType type, const char* ptr,
                                     int length) {
  char buf[kHeaderSize];
  // little end.
  buf[4] = static_cast<char>(length & 0xff);
  buf[5] = static_cast<char>(length >> 8);
  buf[6] = static_cast<char>(type);

  // FIXME: crc32 is zero now.
  uint32_t crc32 = 0;
  EncodeFixed32(buf, 0);

  Status s = dest_->Append(Slice(buf, kHeaderSize));
  if (s.ok()) {
    s = dest_->Append(Slice(ptr, length));
    if (s.ok()) {
      dest_->Flush();
    }
  }
  block_offset_ += length + kHeaderSize;

  return s;
}
};  // namespace log
}  // namespace leveldb
