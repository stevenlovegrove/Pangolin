#pragma once

#include "FileInterface.h"

#include <thread>
#include <mutex>
#include <functional>
#include <fstream>
#include <queue>
#include <cstring>
#include <condition_variable>

#include <mio/mmap.hpp>
#include <pangolin/utils/file_utils.h>

namespace pangolin
{

// A thread safe queued file wrapper
class MemoryMappedFile : public FileInterface
{
public:
    static constexpr size_t block_size = 4096;
    static constexpr size_t min_spare_bytes = block_size * 1000000;

    MemoryMappedFile(const MemoryMappedFile&) = delete;

    MemoryMappedFile(const std::string& filename, bool read_only = false)
        : filename(filename), fd(mio::invalid_handle), read_only(read_only)
    {
        std::error_code err;
        fd = ::open(filename.c_str(), O_RDWR | O_DIRECT | O_CREAT, 0644);
        file_size = mio::detail::query_file_size(fd, err);
        if(err) throw std::runtime_error(err.message());
        file_data_end = file_size;
    }

    ~MemoryMappedFile()
    {
        if(file_data_end < file_size) {
            Truncate(file_data_end);
        }
        ::close(fd);
    }

    void Clear() override
    {
        std::unique_lock<std::mutex> l(map_mutex);
        file_data_end = 0;
    }

    void Put(const uint8_t* data, size_t size_bytes, ssize_t offset = -1 ) override
    {
        auto m = GetMapped(size_bytes, offset);
        std::memcpy(m.get(), data, size_bytes);
    }

    void PutAsync(const std::shared_ptr<uint8_t>& data, size_t size_bytes, ssize_t offset = -1) override
    {
        Put(data.get(), size_bytes, offset);
    }

    void Get(uint8_t* dst, size_t size_bytes, size_t file_offset_bytes) override
    {
        auto data = GetMapped(size_bytes, file_offset_bytes);
        std::memcpy(dst, data.get(), size_bytes);
    }

    std::shared_ptr<uint8_t>
    GetMapped(size_t size_bytes, size_t offset_bytes)  override
    {
        std::unique_lock<std::mutex> l(map_mutex);

        if(offset_bytes == -1) offset_bytes = file_data_end;

        const size_t write_end = offset_bytes + size_bytes;
        const size_t map_end = mmap ? mmap->offset() + mmap->length() : 0;

        if(map_end < write_end) {
            std::error_code err;

            if(file_size < write_end) {
                Truncate(write_end + min_spare_bytes);
            }

            mmap = std::make_shared<mio::ummap_sink>();
            mmap->map(fd, 0, mio::map_entire_file, err);
            if(err) throw std::runtime_error(err.message());
        }

        file_data_end = std::max(file_data_end, write_end);

        uint8_t* user_ptr =  mmap->data() + offset_bytes;
        return std::shared_ptr<uint8_t>(mmap, user_ptr);
    }

private:
    void Truncate(size_t size_bytes )
    {
        ftruncate(fd, size_bytes);
        file_size = size_bytes;
    }

    // Seperate file descriptor for memory mapped reading.
    std::string filename;
    mio::file_handle_type fd;
    std::shared_ptr<mio::ummap_sink> mmap;
    size_t file_data_end;
    size_t file_size;
    bool read_only;

    std::mutex map_mutex;
};

}
