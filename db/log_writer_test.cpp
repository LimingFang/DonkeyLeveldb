#include "gtest/gtest.h"
#include "db/log_writer.h"
#include "leveldb/env.h"
#include <cstdlib>

namespace leveldb {
  const int kMockFileContent = 16 << 20;
  class MockWritableFile : public WritableFile{
    public:
      MockWritableFile() {
        std::memset(content_,0,sizeof content_);
      }

      Status Append(const Slice& data) {
        std::memcpy(&content_[ptr_],data.data(),data.size());
        ptr_ += data.size();
        return Status::OK();
      }

      Status Close() { return Status::OK(); }

      Status Flush() { return Status::OK(); }

      Status Sync() { return Status::OK(); }
      
      char content_[kMockFileContent];
      int ptr_ = 0;
  };

  namespace log {
    
    class LogWriterTest : public testing::Test {
      public:
        LogWriterTest() {
          file_ = new MockWritableFile();
          logwriter_ = new LogWriter(file_);
        }

        ~LogWriterTest() {
          delete file_;
          delete logwriter_;
        }

      protected:
        LogWriter* logwriter_;
        MockWritableFile* file_;
    };

    TEST_F(LogWriterTest,TestOne) {
      int record_length_one = 12;
      int record_length_two = 14;
      char c = 'c';

      Slice record_one(std::string(record_length_one,c));
      Slice record_two(std::string(record_length_two,c));
      logwriter_->AddRecord(&record_one);
      logwriter_->AddRecord(&record_two);

      uint32_t actual_crc;
      uint16_t actual_length;
      RecordType actual_type;
      Slice actual_content;
      // first record
      char* current_ptr = file_->content_;
      // crc
      std::memcpy(&actual_crc,current_ptr,sizeof(actual_crc));
      EXPECT_EQ(0,actual_crc);
      current_ptr += sizeof(actual_crc);
      // length
      std::memcpy(&actual_length,current_ptr,sizeof(actual_length));
      ASSERT_EQ(record_one,actual_length);
      current_ptr += sizeof(actual_length);
      // type
      std::memcpy(&actual_type,current_ptr,sizeof(actual_type));
      EXPECT_EQ(RecordType::kFullType,actual_type);
      current_ptr += sizeof(actual_type);
      // content
      actual_content = Slice(current_ptr,actual_length);
      current_ptr += actual_length;
      EXPECT_EQ(record_one,actual_content);

      // second record
      // crc
      std::memcpy(&actual_crc,current_ptr,sizeof(actual_crc));
      EXPECT_EQ(0,actual_crc);
      current_ptr += sizeof(actual_crc);
      // length
      std::memcpy(&actual_length,current_ptr,sizeof(actual_length));
      ASSERT_EQ(record_one,actual_length);
      current_ptr += sizeof(actual_length);
      // type
      std::memcpy(&actual_type,current_ptr,sizeof(actual_type));
      EXPECT_EQ(RecordType::kFullType,actual_type);
      current_ptr += sizeof(actual_type);
      // content
      actual_content = Slice(current_ptr,actual_length);
      EXPECT_EQ(record_one,actual_content);
    }

    TEST_F(LogWriterTest,TestTwo) {

    }

    TEST_F(LogWriterTest,TestThree) {

    }
  };
}