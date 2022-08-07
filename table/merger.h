//
// Created by 方里明 on 2022/8/7.
//

#ifndef DONKEYLEVELDB_CMAKE_BUILD_DEBUG_MERGER_H_
#define DONKEYLEVELDB_CMAKE_BUILD_DEBUG_MERGER_H_

#include <vector>

namespace leveldb {

class Iterator;
class Comparator;

Iterator* NewMergingIterator(Comparator* comparator,
                             std::vector<Iterator*> children);

}  // namespace leveldb

#endif  // DONKEYLEVELDB_CMAKE_BUILD_DEBUG_MERGER_H_
