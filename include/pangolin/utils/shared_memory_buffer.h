#pragma once

#include <pangolin/utils/semaphore.h>

#include <cstdint>
#include <memory>
#include <string.h>

namespace pangolin
{
  class SharedMemoryBufferInterface
  {
  public:
    virtual ~SharedMemoryBufferInterface() {
    }
    virtual bool tryLock() = 0;
    virtual void lock() = 0;
    virtual void unlock() = 0;
    virtual uint8_t *ptr() = 0;
    virtual std::string name() = 0;
  };

  std::unique_ptr<SharedMemoryBufferInterface> create_named_shared_memory_buffer(const
    std::string& name, size_t size);
  std::unique_ptr<SharedMemoryBufferInterface> open_named_shared_memory_buffer(const
    std::string& name, bool readwrite);
}
