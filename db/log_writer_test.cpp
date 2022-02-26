#include "db/log_writer.h"

#include <cstdlib>

#include "gtest/gtest.h"
#include "leveldb/env.h"

namespace leveldb {
const int kMockFileContent = 16 << 20;
class MockWritableFile : public WritableFile {
 public:
  MockWritableFile() { std::memset(content_, 0, sizeof content_); }

  ~MockWritableFile(){};

  Status Append(const Slice& data) {
    std::memcpy(&content_[ptr_], data.data(), data.size());
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

TEST_F(LogWriterTest, TestOne) {
  Slice bkey("key" + std::to_string(1));
  Slice bval("val" + std::to_string(1));
  std::cout << bkey.data() << std::endl;
  std::cout << bkey.ToString() << std::endl;
  std::cout << bval.ToString() << std::endl;
  assert(bkey.ToString() == std::string("key0"));
  assert(bval.ToString() == std::string("val0"));
  int record_length_one = 12;
  int record_length_two = 14;
  char c = 'c';

  Slice record_one(std::string(record_length_one, c));
  Slice record_two(std::string(record_length_two, c));
  logwriter_->AddRecord(&record_one);
  logwriter_->AddRecord(&record_two);

  uint32_t actual_crc;
  uint16_t actual_length;
  RecordType actual_type;
  Slice actual_content;
  // first record
  char* current_ptr = file_->content_;
  // crc
  std::memcpy(&actual_crc, current_ptr, sizeof(actual_crc));
  EXPECT_EQ(0, actual_crc);
  current_ptr += sizeof(actual_crc);
  // length
  std::memcpy(&actual_length, current_ptr, sizeof(actual_length));
  ASSERT_EQ(record_length_one, actual_length);
  current_ptr += sizeof(actual_length);
  // type
  std::memcpy(&actual_type, current_ptr, sizeof(actual_type));
  EXPECT_EQ(RecordType::kFullType, actual_type);
  current_ptr += sizeof(actual_type);
  // content
  actual_content = Slice(current_ptr, actual_length);
  current_ptr += actual_length;
  EXPECT_TRUE(record_one == actual_content);

  // second record
  // crc
  std::memcpy(&actual_crc, current_ptr, sizeof(actual_crc));
  EXPECT_EQ(0, actual_crc);
  current_ptr += sizeof(actual_crc);
  // length
  std::memcpy(&actual_length, current_ptr, sizeof(actual_length));
  ASSERT_EQ(record_length_two, actual_length);
  current_ptr += sizeof(actual_length);
  // type
  std::memcpy(&actual_type, current_ptr, sizeof(actual_type));
  EXPECT_EQ(RecordType::kFullType, actual_type);
  current_ptr += sizeof(actual_type);
  // content
  actual_content = Slice(current_ptr, actual_length);
  EXPECT_TRUE(record_two == actual_content);
}

TEST_F(LogWriterTest, TestTwo) {
  // 跨block的record,跨三个个block
  int record_length = 80 << 10;
  int record_first_length = (32 << 10) - kHeaderSize;
  int record_second_length = record_first_length;
  int record_third_length = (record_length - 2 * record_first_length);
  char c = 'c';
  std::string s(record_length, c);
  Slice record(s);
  Slice record_first(s.data(), record_first_length);
  Slice record_second(s.data(), record_second_length);
  Slice record_third(s.data(), record_third_length);
  logwriter_->AddRecord(&record);

  uint32_t actual_crc;
  uint16_t actual_length;
  RecordType actual_type;
  Slice actual_content;
  char* current_ptr = file_->content_;

  // crc
  std::memcpy(&actual_crc, current_ptr, sizeof(actual_crc));
  EXPECT_EQ(0, actual_crc);
  current_ptr += sizeof(actual_crc);
  // length
  std::memcpy(&actual_length, current_ptr, sizeof(actual_length));
  ASSERT_EQ(record_first_length, actual_length);
  current_ptr += sizeof(actual_length);
  // type
  std::memcpy(&actual_type, current_ptr, sizeof(actual_type));
  EXPECT_EQ(RecordType::kFirstType, actual_type);
  current_ptr += sizeof(actual_type);
  // content
  actual_content = Slice(current_ptr, record_first_length);
  current_ptr += record_first_length;
  EXPECT_TRUE(record_first == actual_content);
  // ---
  // crc
  std::memcpy(&actual_crc, current_ptr, sizeof(actual_crc));
  EXPECT_EQ(0, actual_crc);
  current_ptr += sizeof(actual_crc);
  // length
  std::memcpy(&actual_length, current_ptr, sizeof(actual_length));
  ASSERT_EQ(record_second_length, actual_length);
  current_ptr += sizeof(actual_length);
  // type
  std::memcpy(&actual_type, current_ptr, sizeof(actual_type));
  EXPECT_EQ(RecordType::kMidType, actual_type);
  current_ptr += sizeof(actual_type);
  // content
  actual_content = Slice(current_ptr, record_second_length);
  current_ptr += record_second_length;
  EXPECT_TRUE(record_second == actual_content);
  // ---
  // crc
  std::memcpy(&actual_crc, current_ptr, sizeof(actual_crc));
  EXPECT_EQ(0, actual_crc);
  current_ptr += sizeof(actual_crc);
  // length
  std::memcpy(&actual_length, current_ptr, sizeof(actual_length));
  ASSERT_EQ(record_third_length, actual_length);
  current_ptr += sizeof(actual_length);
  // type
  std::memcpy(&actual_type, current_ptr, sizeof(actual_type));
  EXPECT_EQ(RecordType::kLastType, actual_type);
  current_ptr += sizeof(actual_type);
  // content
  actual_content = Slice(current_ptr, record_third_length);
  EXPECT_TRUE(record_third == actual_content);
}

TEST_F(LogWriterTest, TestThree) {
  int record_length_one = (32 << 10) - kHeaderSize - 3;
  int record_length_two = 14;
  char c = 'c';

  Slice record_one(std::string(record_length_one, c));
  Slice record_two(std::string(record_length_two, c));
  logwriter_->AddRecord(&record_one);
  logwriter_->AddRecord(&record_two);

  uint32_t actual_crc;
  uint16_t actual_length;
  RecordType actual_type;
  Slice actual_content;
  // first record
  char* current_ptr = file_->content_;
  // crc
  std::memcpy(&actual_crc, current_ptr, sizeof(actual_crc));
  EXPECT_EQ(0, actual_crc);
  current_ptr += sizeof(actual_crc);
  // length
  std::memcpy(&actual_length, current_ptr, sizeof(actual_length));
  ASSERT_EQ(record_length_one, actual_length);
  current_ptr += sizeof(actual_length);
  // type
  std::memcpy(&actual_type, current_ptr, sizeof(actual_type));
  EXPECT_EQ(RecordType::kFullType, actual_type);
  current_ptr += sizeof(actual_type);
  // content
  actual_content = Slice(current_ptr, actual_length);
  current_ptr += actual_length;
  EXPECT_TRUE(record_one == actual_content);

  // second record
  current_ptr += 3;
  // crc
  std::memcpy(&actual_crc, current_ptr, sizeof(actual_crc));
  EXPECT_EQ(0, actual_crc);
  current_ptr += sizeof(actual_crc);
  // length
  std::memcpy(&actual_length, current_ptr, sizeof(actual_length));
  ASSERT_EQ(record_length_two, actual_length);
  current_ptr += sizeof(actual_length);
  // type
  std::memcpy(&actual_type, current_ptr, sizeof(actual_type));
  EXPECT_EQ(RecordType::kFullType, actual_type);
  current_ptr += sizeof(actual_type);
  // content
  actual_content = Slice(current_ptr, actual_length);
  EXPECT_TRUE(record_two == actual_content);
}
};  // namespace log
}  // namespace leveldb

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}