#include "leveldb/env.h"

#include <fcntl.h>
#include <string_view>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#include "leveldb/status.h"

#include "util/env_posix_test_helper.h"
#include "util/logger.h"

namespace leveldb {

constexpr const size_t kWritableFileBufferSize = 65536;
constexpr const int kOpenBaseFlags = O_CLOEXEC;
// Set by EnvPosixTestHelper::SetReadOnlyMMapLimit() and MaxOpenFiles().
int g_open_read_only_file_limit = -1;

// Up to 1000 mmap regions for 64-bit binaries; none for 32-bit.
constexpr const int kDefaultMmapLimit = (sizeof(void*) >= 8) ? 1000 : 0;

// Can be set using EnvPosixTestHelper::SetReadOnlyMMapLimit().
int g_mmap_limit = kDefaultMmapLimit;

Status PosixError(std::string_view context, int error_number) {
  if (error_number == ENOENT) {
    return Status::NotFound(context, std::strerror(error_number));
  } else {
    return Status::IOError(context, std::strerror(error_number));
  }
}

void Log(Logger* info_log, const char* format, ...) {
  if (info_log != nullptr) {
    std::va_list ap;
    va_start(ap, format);
    info_log->Logv(format, ap);
    va_end(ap);
  }
}

static Status DoWriteStringToFile(Env* env, std::string_view data,
                                  std::string_view fname, bool should_sync) {
  WritableFile* file;
  Status s = env->NewWritableFile(fname, &file);
  if (!s.ok()) {
    return s;
  }
  s = file->Append(data);
  if (s.ok() && should_sync) {
    s = file->Sync();
  }
  if (s.ok()) {
    s = file->Close();
  }
  delete file;  // Will auto-close if we did not close above
  if (!s.ok()) {
    env->RemoveFile(fname);
  }
  return s;
}

Status WriteStringToFile(Env* env, std::string_view data,
                         std::string_view fname) {
  return DoWriteStringToFile(env, data, fname, false);
}

Status WriteStringToFileSync(Env* env, std::string_view data,
                             std::string_view fname) {
  return DoWriteStringToFile(env, data, fname, true);
}

Status ReadFileToString(Env* env, std::string_view fname, std::string* data) {
  data->clear();
  SequentialFile* file;
  Status s = env->NewSequentialFile(fname, &file);
  if (!s.ok()) {
    return s;
  }
  static const int kBufferSize = 8192;
  char* space = new char[kBufferSize];
  while (true) {
    std::string_view fragment;
    s = file->Read(kBufferSize, &fragment, space);
    if (!s.ok()) {
      break;
    }
    data->append(fragment.data(), fragment.size());
    if (fragment.empty()) {
      break;
    }
  }
  delete[] space;
  delete file;
  return s;
}

Limiter::Limiter(int max_acquires)
    :
#if !defined(NDEBUG)
      max_acquires_(max_acquires),
#endif  // !defined(NDEBUG)
      acquires_allowed_(max_acquires) {
  assert(max_acquires >= 0);
}

bool Limiter::Acquire() {
  int old_acquires_allowed =
      acquires_allowed_.fetch_sub(1, std::memory_order_relaxed);

  if (old_acquires_allowed > 0) return true;

  int pre_increment_acquires_allowed =
      acquires_allowed_.fetch_add(1, std::memory_order_relaxed);

  // Silence compiler warnings about unused arguments when NDEBUG is defined.
  (void)pre_increment_acquires_allowed;
  // If the check below fails, Release() was called more times than acquire.
  assert(pre_increment_acquires_allowed < max_acquires_);

  return false;
}

void Limiter::Release() {
  int old_acquires_allowed =
      acquires_allowed_.fetch_add(1, std::memory_order_relaxed);

  // Silence compiler warnings about unused arguments when NDEBUG is defined.
  (void)old_acquires_allowed;
  // If the check below fails, Release() was called more times than acquire.
  assert(old_acquires_allowed < max_acquires_);
}

// Return the maximum number of concurrent mmaps.
int MaxMmaps() { return g_mmap_limit; }

// Return the maximum number of read-only files to keep open.
int MaxOpenFiles() {
  if (g_open_read_only_file_limit >= 0) {
    return g_open_read_only_file_limit;
  }
  struct ::rlimit rlim;
  if (::getrlimit(RLIMIT_NOFILE, &rlim)) {
    // getrlimit failed, fallback to hard-coded default.
    g_open_read_only_file_limit = 50;
  } else if (rlim.rlim_cur == RLIM_INFINITY) {
    g_open_read_only_file_limit = std::numeric_limits<int>::max();
  } else {
    // Allow use of 20% of available file descriptors for read-only files.
    g_open_read_only_file_limit = rlim.rlim_cur / 5;
  }
  return g_open_read_only_file_limit;
}

SequentialFile::SequentialFile(std::string_view fname, int fd)
    : fname_(fname), fd_(fd) {}

SequentialFile::~SequentialFile() {
  if (fd_ >= 0) {
    // Ignoring any potential errors
    ::close(fd_);
  }
}

Status SequentialFile::Read(size_t n, std::string_view* result, char* scratch) {
  while (true) {
    int m = ::read(fd_, scratch, n);
    if (m < 0) {
      if (errno == EINTR) {
        continue;
      }
      return PosixError(fname_, errno);
    }
    *result = std::string_view(scratch, m);
    break;
  }
  return Status::OK();
}

Status SequentialFile::Skip(size_t n) {
  // seek file offset based of current offset.
  int curr = ::lseek(fd_, n, SEEK_CUR);
  if (curr < 0) {
    return PosixError(fname_, errno);
  }
  return Status::OK();
}

Status RandomAccessFile::Read(size_t offset, size_t n, std::string_view* result,
                              char* scratch) {
  while (true) {
    int m = ::pread(fd_, scratch, n, offset);
    if (m < 0) {
      if (errno == EINTR) {
        continue;
      }
      return PosixError(fname_, errno);
    }
    *result = std::string_view(scratch, m);
    break;
  }
  return Status::OK();
}

WritableFile::WritableFile(std::string_view filename, int fd)
    : fd_(fd), fname_(filename) {
  buffer_ = new char[kWritableFileBufferSize];
}

WritableFile::~WritableFile() { delete[] buffer_; }

// TODO(fangliming.xxm): impl it.
Status WritableFile::FlushBuffer() { return Status::OK(); }
Status WritableFile::WriteUnbuffered(const char* data, size_t size) {
  auto done = 0;
  while (done < size) {
    int m = ::write(fd_, data + done, size - done);
    if (m < 0) {
      if (errno == EINTR) {
        continue;
      }
      return PosixError(fname_, errno);
    }
    done += m;
  }
  return Status::OK();
}

Status WritableFile::Append(std::string_view data) {
  auto sz = data.size();
  auto write_data = data.data();

  // TODO(fangliming.xxm): refine
  return WriteUnbuffered(write_data, sz);
}
Status WritableFile::Close() {
  if (::close(fd_) < 0) {
    return PosixError(fname_, errno);
  }
  return Status::OK();
}
Status WritableFile::Flush() { return FlushBuffer(); }
Status WritableFile::Sync() {
  // TODO(fangliming.xxm): consider manifest.
  auto st = FlushBuffer();
  if (!st.ok()) {
    return st;
  }
  if (::fsync(fd_) != 0) {
    return PosixError(fname_, errno);
  }
  return Status::OK();
}

RandomAccessFile::RandomAccessFile(std::string_view fname, int fd)
    : fname_(fname), fd_(fd) {}

RandomAccessFile::~RandomAccessFile() {
  if (fd_ >= 0) {
    // Ignoring any potential errors
    ::close(fd_);
  }
}

Env::Env() : mmap_limiter_(MaxMmaps()), fd_limiter_(MaxOpenFiles()) {}

Env* Env::Default() {
  static Env env;
  return &env;
}

bool Env::FileExists(std::string_view fname) {
  return ::access(fname.data(), F_OK) == 0;
}

Status Env::GetChildren(std::string_view dir,
                        std::vector<std::string>* result) {
  return Status::OK();
}

Status Env::RemoveFile(std::string_view fname) {
  if (::unlink(fname.data()) != 0) {
    return PosixError(fname, errno);
  }
  return Status::OK();
}

Status Env::DeleteFile(std::string_view fname) { return Status::OK(); }

Status Env::CreateDir(std::string_view dirname) {
  if (::mkdir(dirname.data(), 0755) != 0) {
    return PosixError(dirname, errno);
  }
  return Status::OK();
}

Status Env::RemoveDir(std::string_view dirname) {
  if (::rmdir(dirname.data()) != 0) {
    return PosixError(dirname, errno);
  }
  return Status::OK();
}

Status Env::RenameFile(std::string_view src, std::string_view target) {
  if (::rename(src.data(), target.data()) != 0) {
    return PosixError(src, errno);
  }
  return Status::OK();
}

Status Env::GetFileSize(std::string_view fname, uint64_t* file_size) {
  struct stat st;
  if (::stat(fname.data(), &st) != 0) {
    return PosixError(fname, errno);
  }
  *file_size = st.st_size;
  return Status::OK();
}

Status Env::NewLogger(std::string_view filename, Logger** result) {
  int fd = ::open(filename.data(),
                  O_APPEND | O_WRONLY | O_CREAT | kOpenBaseFlags, 0644);
  if (fd < 0) {
    *result = nullptr;
    return PosixError(filename, errno);
  }

  std::FILE* fp = ::fdopen(fd, "w");
  if (fp == nullptr) {
    ::close(fd);
    *result = nullptr;
    return PosixError(filename, errno);
  } else {
    *result = new Logger(fp);
    return Status::OK();
  }
}

uint64_t Env::NowMicros() {
  static constexpr uint64_t kUsecondsPerSecond = 1000000;
  struct ::timeval tv;
  ::gettimeofday(&tv, nullptr);
  return static_cast<uint64_t>(tv.tv_sec) * kUsecondsPerSecond + tv.tv_usec;
}

void Env::SleepForMicroseconds(int micros) {
  std::this_thread::sleep_for(std::chrono::microseconds{micros});
}

Status Env::NewSequentialFile(std::string_view fname, SequentialFile** result) {
  int fd = ::open(fname.data(), O_RDONLY | O_CLOEXEC);
  if (fd < 0) {
    *result = nullptr;
    return PosixError(fname, errno);
  }
  *result = new SequentialFile(fname, fd);
  return Status::OK();
}

Status Env::NewRandomAccessFile(std::string_view fname,
                                RandomAccessFile** result) {
  int fd = ::open(fname.data(), O_RDONLY | O_CLOEXEC);
  if (fd < 0) {
    *result = nullptr;
    return PosixError(fname, errno);
  }
  *result = new RandomAccessFile(fname, fd);
  return Status::OK();
}

Status Env::NewWritableFile(std::string_view fname, WritableFile** result) {
  int fd = ::open(fname.data(), O_CREAT | O_WRONLY | O_CLOEXEC | O_TRUNC, 0644);
  if (fd < 0) {
    *result = nullptr;
    return PosixError(fname, errno);
  }
  *result = new WritableFile(fname, fd);
  return Status::OK();
}

Status Env::GetTestDirectory(std::string* result) {
  const char* env = std::getenv("TEST_TMPDIR");
  if (env && env[0] != '\0') {
    *result = env;
  } else {
    char buf[100];
    std::snprintf(buf, sizeof(buf), "/tmp/leveldbtest-%d",
                  static_cast<int>(::geteuid()));
    *result = buf;
  }

  // The CreateDir status is ignored because the directory may already exist.
  CreateDir(*result);

  return Status::OK();
}

void EnvPosixTestHelper::SetReadOnlyFDLimit(int limit) {
  g_open_read_only_file_limit = limit;
}

void EnvPosixTestHelper::SetReadOnlyMMapLimit(int limit) {
  g_mmap_limit = limit;
}

}  // namespace leveldb