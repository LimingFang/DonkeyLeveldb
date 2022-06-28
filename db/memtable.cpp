//
// Created by 方里明 on 3/1/22.
//

#include "memtable.h"

#include "db/db_format.h"

#include "leveldb/iterator.h"
#include "leveldb/slice.h"
#include "leveldb/status.h"

#include "util/arena.h"
#include "util/coding.h"
namespace leveldb {

Slice GetLengthPrefixedSlice(const char* data) { return Slice(); }

MemTable::MemTable(const InternalKeyComparator& comparator)
    : comparator_(comparator),
      arena_(),
      table_(comparator_, &arena_),
      refs_(0) {}

void leveldb::MemTable::Add(leveldb::SequenceNumberType seq,
                            leveldb::ValueType type, const leveldb::Slice& key,
                            const leveldb::Slice& value) {
  int key_size = key.size();
  int val_size = value.size();
  int internal_key_size = key_size + 8;
  // 1.allocate space
  int encoded_length = VariantLength(internal_key_size) + internal_key_size +
                       VariantLength(val_size) + val_size;
  char* c = arena_.Allocate(encoded_length);
  char* ptr = c;
  // 2.Encode key_length,key,tag,val_length,val in sequence.
  c = EncodeVariant(c, internal_key_size);
  std::memcpy(c, key.data(), key_size);
  c += key_size;
  c = EncodeFixed64(c, (seq << 8) | type);
  c = EncodeFixed64(c, val_size);
  std::memcpy(c, value.data(), val_size);
  // 3.insert into table(skiplist).
  table_.Insert(ptr);
}

bool MemTable::Get(const LookupKey& key, std::string* value, Status* st) {
  auto memtable_key = key.memtable_key();
  auto iter = Table::Iterator(&table_);
  iter.SeekTo(memtable_key.data());
  if (iter.Valid()) {
    // 能找到相应 entry{InternalKey >= our InternalKey}
    Slice entry = iter.Value();
    uint32_t user_key_length;
    auto user_key_start =
        GetVariant32Ptr(entry.data(), entry.data() + 5, &user_key_length);
    if (comparator_.comparator.user_comparator()->Compare(
            Slice(user_key_start, user_key_length - 8), key.user_key()) == 0) {
      // user key 相同，比对 tag
      const uint64_t tag = DecodeFixed64(user_key_start + user_key_length - 8);
      switch (static_cast<ValueType>(tag & 0xff)) {
        case kTypeValue: {
          Slice v = GetLengthPrefixedSlice(user_key_start + user_key_length);
          value->assign(v.data(), v.size());
          *st = Status::OK();
          return true;
        }
        case kTypeDeletion: {
          *st = Status::NotFound(Slice());
          return true;
        }
      }
    }
  }
  return false;
}

class MemTableIterator : public Iterator {
 public:
  explicit MemTableIterator(MemTable::Table* table) : iter_(table) {}

  virtual ~MemTableIterator() override = default;

  bool Valid() const override { return iter_.Valid(); };

  void Next() override { iter_.Next(); }

  Slice Key() override;

  Slice Value() const override;

  void SeekToLast() override { iter_.SeekToLast(); }

  void SeekToFirst() override { iter_.SeekToFirst(); }

  void SeekTo(const Slice& slice) override {  // iter_.SeekTo(slice);
  }

 private:
  MemTable::Table::Iterator iter_;
};

Iterator* MemTable::NewIterator() { return new MemTableIterator(&table_); }

}  // namespace leveldb
