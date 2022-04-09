//
// Created by 方里明 on 3/1/22.
//

#include "memtable.h"

#include "leveldb/iterator.h"
#include "util/coding.h"
namespace leveldb {
void leveldb::MemTable::Add(leveldb::SequenceNumberType seq,
                            leveldb::ValueType type, const leveldb::Slice& key,
                            const leveldb::Slice& value) {
  int key_size = key.size();
  int val_size = value.size();
  int internal_key_size = key_size + 8;
  // 1.allocate space
  int encoded_length = VariantLength(internal_key_size) + internal_key_size +
                       VariantLength(val_size) + val_size;
  char* c = arena_->Allocate(encoded_length);
  char* ptr = c;
  // 2.Encode key_length,key,tag,val_length,val in sequence.
  c = EncodeVariant(c, internal_key_size);
  std::memcpy(c, key.data(), key_size);
  c += key_size;
  c = EncodeFixed64(c, (seq << 8) | type);
  c = EncodeFixed64(c, val_size);
  std::memcpy(c, value.data(), val_size);
  // 3.insert into table(skiplist).
  table_->Insert(ptr);
}

bool MemTable::Get(const LookupKey& key, std::string* value, Status* st) {}

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

  void SeekTo(const Slice& slice) override { iter_.SeekTo(slice); }

 private:
  MemTable::Table::Iterator iter_;
};
}  // namespace leveldb
