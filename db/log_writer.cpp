#include "log_writer.h"
#include "log_format.h"
#include "leveldb/env.h"
#include "util/crc32c.h"
#include "util/coding.h"


namespace leveldb {
namespace log {
    // 将slice写入当前的file中，内部会处理好block和record的关系
    Status LogWriter::AddRecord(const Slice* slice) {
        size_t left = slice->size();
        bool begin = true;
        const char* ptr = slice->data();
				Status s;

        do {
            // 1.如果当前block放不下，则开一个新的block，写入
            // 旧的block全写0
            int block_remain = kBlockSize - block_offset_;
            if (block_remain < kHeaderSize) {
                if(block_remain>0) {
                    dest_->Append(Slice("\x00\x00\x00\x00\x00\x00\x00",block_remain));
                }
                block_offset_ = 0;
                block_remain = kBlockSize;
            }

						int fragment_length = (left>block_remain)?block_remain:left;
            left -= fragment_length;
            RecordType type;
            bool end = (left == 0);
            if(begin && end) {
                type = kFullType;
            } else if(begin && !end) {
                type = kFirstType;
            } else if(!begin && end) {
                type = kMidType;
            } else {
              type = kLastType;
            }

          s = EmitPhysicalRecord(type,ptr,fragment_length);
					begin = false;
					block_offset_ += fragment_length;
					ptr += fragment_length;
        } while(s.ok() && left>0);

        return s;
    }

    // 将给定的content按照log format写入file
    Status LogWriter::EmitPhysicalRecord(RecordType type,const char* ptr,int length) {
        char buf[kHeaderSize];
        // little end.
        buf[4] = static_cast<char>(length & 0xff);
        buf[5] = static_cast<char>(length >> 8);
        buf[6] = static_cast<char>(type);

        // FIXME: crc32 is zero now.
        uint32_t crc32 = 0;
        EncodeFixed32(buf,0);

        Status s = dest_->Append(Slice(buf,kHeaderSize));
        if(s.ok()) {
            s = dest_->Append(Slice(ptr,length));
            if(s.ok()) {
                dest_->Flush();
            }
        }
        block_offset_ += length + kHeaderSize;

        return s;
    }
};
}