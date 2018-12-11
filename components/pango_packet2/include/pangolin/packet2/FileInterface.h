#pragma once

#include <memory>

namespace pangolin
{

class FileInterface
{
  public:
    virtual ~FileInterface() {}

    virtual void Clear() = 0;

    virtual void Put(const uint8_t* data, size_t size_bytes, ssize_t offset = -1 ) = 0;

    virtual void PutAsync(const std::shared_ptr<uint8_t>& data, size_t size_bytes, ssize_t offset = -1 ) = 0;

    virtual void Get(uint8_t* dst, size_t size_bytes, size_t file_offset_bytes) = 0;

    virtual std::shared_ptr<uint8_t> GetMapped(size_t size_bytes, size_t offset_bytes) = 0;
};

}
