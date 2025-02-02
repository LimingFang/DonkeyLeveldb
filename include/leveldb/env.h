#pragma once
#include <string_view>
#include <vector>

#include "leveldb/status.h"

namespace leveldb {

// File for sequential read.
class SequentialFile {
 public:
  SequentialFile(std::string_view filename, int fd);

  SequentialFile(const SequentialFile&) = delete;
  SequentialFile& operator=(const SequentialFile&) = delete;

  ~SequentialFile();

  // Read at most n bytes, result will point to that area, these data may or may
  // or be read into scratch, so scratch should be alive when result is used.
  //
  // REQUIRES: external synchronization
  Status Read(size_t n, std::string_view* result, char* scratch);

  Status Skip(size_t n);

 private:
  std::string fname_;
  int fd_;
};

// File for random read.
class RandomAccessFile {
 public:
  RandomAccessFile(std::string_view filename, int fd);

  RandomAccessFile(const RandomAccessFile&) = delete;
  RandomAccessFile& operator=(const RandomAccessFile&) = delete;

  ~RandomAccessFile();

  // Read at most n bytes, result will point to that area, these data may or may
  // or be read into scratch, so scratch should be alive when result is used.
  Status Read(size_t offset, size_t n, std::string_view* result, char* scratch);

 private:
  std::string fname_;
  int fd_;
};

// File for writing.
class WritableFile {
 public:
  WritableFile(std::string_view filename, int fd);

  WritableFile(const WritableFile&) = delete;
  WritableFile& operator=(const WritableFile&) = delete;

  ~WritableFile();

  Status Append(std::string_view data);
  Status Close();
  Status Flush();
  Status Sync();

 private:
  Status FlushBuffer();
  Status WriteUnbuffered(const char* data, size_t size);

 private:
  int fd_;
  std::string fname_;
  char* buffer_;
  int pos_;
};

// Helper class to limit resource usage to avoid exhaustion.
// Currently used to limit read-only file descriptors and mmap file usage
// so that we do not run out of file descriptors or virtual memory, or run into
// kernel performance problems for very large databases.
class Limiter {
 public:
  // Limit maximum number of resources to |max_acquires|.
  Limiter(int max_acquires);

  Limiter(const Limiter&) = delete;
  Limiter operator=(const Limiter&) = delete;

  // If another resource is available, acquire it and return true.
  // Else return false.
  bool Acquire();

  // Release a resource acquired by a previous call to Acquire() that returned
  // true.
  void Release();

 private:
#if !defined(NDEBUG)
  // Catches an excessive number of Release() calls.
  const int max_acquires_;
#endif  // !defined(NDEBUG)

  // The number of available resources.
  //
  // This is a counter and is not tied to the invariants of any other class, so
  // it can be operated on safely using std::memory_order_relaxed.
  std::atomic<int> acquires_allowed_;
};

class Logger;
class FileLock;
// posix only
class Env {
 public:
  Env();
  Env(const Env&) = delete;
  Env& operator=(const Env&) = delete;

  static Env* Default();

  Status NewSequentialFile(std::string_view fname, SequentialFile** result);
  Status NewRandomAccessFile(std::string_view fname, RandomAccessFile** result);
  Status NewWritableFile(std::string_view fname, WritableFile** result);

  bool FileExists(std::string_view fname);
  Status GetChildren(std::string_view dir, std::vector<std::string>* result);
  Status RemoveFile(std::string_view fname);
  Status DeleteFile(std::string_view fname);
  Status CreateDir(std::string_view dirname);
  Status RemoveDir(std::string_view dirname);
  Status GetFileSize(std::string_view fname, uint64_t* file_size);
  Status RenameFile(std::string_view src, std::string_view target);
  // TODO(fangliming.xxm): impl them.
  Status LockFile(std::string_view fname, FileLock** lock) {
    return Status::OK();
  }
  Status UnlockFile(FileLock* lock) { return Status::OK(); }
  // TODO(fangliming.xxm): impl them.
  void Schedule(void (*function)(void* arg), void* arg) { return; }
  void StartThread(void (*function)(void* arg), void* arg) { return; }

  Status GetTestDirectory(std::string* path);

  Status NewLogger(std::string_view fname, Logger** result);

  uint64_t NowMicros();
  void SleepForMicroseconds(int micros);

 private:
  Limiter mmap_limiter_;  // Thread-safe.
  Limiter fd_limiter_;    // Thread-safe.
};

// Log the specified data to *info_log if info_log is non-null.
void Log(Logger* info_log, const char* format, ...)
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((__format__(__printf__, 2, 3)))
#endif
    ;

// A utility routine: write "data" to the named file.
Status WriteStringToFile(Env* env, std::string_view data,
                         std::string_view fname);

// A utility routine: read contents of named file into *data
Status ReadFileToString(Env* env, std::string_view fname, std::string* data);

}  // namespace leveldb
