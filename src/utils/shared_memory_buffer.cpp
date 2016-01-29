#include <pangolin/utils/shared_memory_buffer.h>

#include <pangolin/utils/semaphore.h>

#include <fcntl.h>
#include <limits.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

namespace pangolin
{

// TODO(shaheen) register a signal handler for SIGTERM and unlink shared memory
// objects.
class PosixSharedMemoryBuffer : public SharedMemoryBufferInterface
{
public:
  PosixSharedMemoryBuffer(int fd, uint8_t *ptr, size_t size, bool ownership, const std::string& name) :
    _fd(fd),
    _ptr(ptr),
    _size(size),
    _ownership(ownership),
    _name(name),
    _lockCount(0)
  {
  }

  ~PosixSharedMemoryBuffer()
  {
    close(_fd);
    munmap(_ptr, _size);

    if (_ownership) {
      shm_unlink(_name.c_str());
    }
  }

  bool tryLock()
  {
    if (_lockCount == 0) {
      int err = flock(_fd, LOCK_EX|LOCK_NB);
      if (0 == err) {
        _lockCount++;
      }
    }
    return _lockCount != 0;
  }

  void lock()
  {
    if (_lockCount == 0) {
      flock(_fd, LOCK_EX);
    }
    _lockCount++;
  }

  void unlock()
  {
    if (_lockCount != 0) {
      flock(_fd, LOCK_UN);
    }
    _lockCount--;
  }

  uint8_t *ptr()
  {
    return _ptr;
  }

  std::string name()
  {
    return _name;
  }

private:
  int _fd;
  uint8_t *_ptr;
  size_t _size;
  bool _ownership;
  string _name;
  unsigned int _lockCount;
};

unique_ptr<SharedMemoryBufferInterface> create_named_shared_memory_buffer(const
  string& name, size_t size)
{
  unique_ptr<SharedMemoryBufferInterface> ptr;

  int fd = shm_open(name.c_str(), O_RDWR | O_RDONLY | O_CREAT,
    S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
  if (-1 == fd) {
    return ptr;
  }

  int err = ftruncate(fd, size);
  if (-1 == err) {
    shm_unlink(name.c_str());
    return ptr;
  }

  uint8_t *buffer = reinterpret_cast<uint8_t *>(mmap(NULL, size,
      PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0));

  ptr.reset(new PosixSharedMemoryBuffer(fd, buffer, size, true, name));
  return ptr;
}

unique_ptr<SharedMemoryBufferInterface> open_named_shared_memory_buffer(const
  string& name, bool readwrite)
{
  unique_ptr<SharedMemoryBufferInterface> ptr;

  int fd = shm_open(name.c_str(), readwrite ? O_RDWR | O_RDONLY : O_RDONLY, 0);
  if (-1 == fd) {
    return ptr;
  }

  struct stat sbuf;
  int err = fstat(fd, &sbuf);
  if (-1 == err) {
    return ptr;
  }

  size_t size = sbuf.st_size;
  uint8_t *buffer = reinterpret_cast<uint8_t *>(mmap(NULL, size,
      PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0));

  ptr.reset(new PosixSharedMemoryBuffer(fd, buffer, size, false, name));
  return ptr;
}

}
