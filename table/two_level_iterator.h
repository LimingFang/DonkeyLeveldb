//
// Created by 方里明 on 2022/8/7.
//

#ifndef DONKEYLEVELDB_TABLE_TWO_LEVEL_ITERATOR_H_
#define DONKEYLEVELDB_TABLE_TWO_LEVEL_ITERATOR_H_

#include "leveldb/iterator.h"

namespace leveldb {

struct ReadOptions;

std::unique_ptr<Iterator> NewTwoLevelIterator(
    std::unique_ptr<Iterator> index_iter,
    std::unique_ptr<Iterator> (*block_function)(void* args, const ReadOptions&,
                                                const Slice& index_value),
    void* args, const ReadOptions& options);

}  // namespace leveldb

#endif  // DONKEYLEVELDB_TABLE_TWO_LEVEL_ITERATOR_H_
