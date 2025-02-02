// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// Logger implementation that can be shared by all environments
// where enough posix functionality is available.
//
// DonkeyLeveldb project rename PosixLogger to Logger and seperate
// implementation from declaration.

#pragma once

#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <sstream>
#include <sys/time.h>

#include "leveldb/env.h"

namespace leveldb {

class Logger {
 public:
  // Creates a logger that writes to the given file.
  //
  // The Logger instance takes ownership of the file handle.
  explicit Logger(std::FILE* fp) : fp_(fp) { assert(fp != nullptr); }

  ~Logger() { std::fclose(fp_); }

  void Logv(const char* format, std::va_list arguments);

 private:
  std::FILE* const fp_;
};

}  // namespace leveldb
