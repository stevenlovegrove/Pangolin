#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <functional>
#include <fstream>
#include <queue>
#include <condition_variable>

#include <mio/mmap.hpp>
#include <pangolin/utils/file_utils.h>

namespace pangolin
{

// A thread safe queued file wrapper
class RandomFile
{
public:
    RandomFile(const std::string& filename, size_t preallocated_size_bytes=0)
        :should_run(true)
    {
        std::error_code err;

        file.open(filename, std::ios_base::binary);
        fd = mio::detail::open_file(filename, mio::access_mode::write, err);
        if(err) throw std::runtime_error(err.message());

        file_size_bytes = mio::detail::query_file_size(fd,err);
        if(err) throw std::runtime_error(err.message());

        data_size_bytes = file_size_bytes;

        // Make sure we have some room for growth without fragmentation
        if(file_size_bytes < preallocated_size_bytes) {
            Truncate(preallocated_size_bytes);
        }

        file.seekg(0, std::ios_base::beg);
        write_thread = std::thread(&RandomFile::WriteThread, this);
    }

    ~RandomFile()
    {
        should_run = false;
        write_thread.join();
        Truncate(data_size_bytes);
    }

    // Atomically stream to file (and jump the queue)
    void Write(const std::function<void(std::ostream&)>& func)
    {
        std::unique_lock<std::mutex> l(write_mutex);
        func(file);
    }

    void Append(const std::shared_ptr<uint8_t>& data, size_t size_bytes)
    {
        std::unique_lock<std::mutex> lq(queue_mutex);
        to_write.emplace([data, size_bytes, this](){
            std::unique_lock<std::mutex> lw(write_mutex);
            Write(data.get(), size_bytes);
        });
        cond.notify_all();
    }

private:
    void Write(uint8_t* data, size_t size_bytes)
    {
        file.write((char*)data, size_bytes);
    }

    void WriteThread()
    {
        while(true)
        {
            while(should_run && to_write.empty()) {
                std::unique_lock<std::mutex> lq(queue_mutex);
                cond.wait(lq);
            }

            if(should_run) {
                to_write.front()();
                {
                    std::unique_lock<std::mutex> lq(queue_mutex);
                    to_write.pop();
                }
            }
        }
    }

    void Truncate(size_t size_bytes)
    {
        ftruncate(fd, size_bytes);
        file_size_bytes = size_bytes;
        data_size_bytes = std::min(data_size_bytes, size_bytes);
    }

    volatile bool should_run;
    std::thread write_thread;

    std::condition_variable cond;
    std::mutex queue_mutex;
    std::mutex write_mutex;
    std::fstream file;
    mio::file_handle_type fd;
    size_t data_size_bytes;
    size_t file_size_bytes;
    std::queue<std::function<void(void)>> to_write;
};

}
