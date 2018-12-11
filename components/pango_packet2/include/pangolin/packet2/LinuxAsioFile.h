#pragma once

#include "FileInterface.h"

#include <thread>
#include <mutex>
#include <functional>
#include <fstream>
#include <queue>
#include <cstring>
#include <condition_variable>
#include <libaio.h>

#include <mio/mmap.hpp>
#include <pangolin/utils/file_utils.h>

namespace pangolin
{

#define RWF_DSYNC	(/*(__force __kernel_rwf_t)*/ 0x00000002)
#define RWF_SYNC	(/*(__force __kernel_rwf_t)*/ 0x00000004)

// A thread safe queued file wrapper
class LinuxAsioFile : public FileInterface
{
    static constexpr int max_events = 100;
public:

    LinuxAsioFile(const LinuxAsioFile&) = delete;

    LinuxAsioFile(const std::string& filename)
        : filename(filename), fd(mio::invalid_handle), end_bytes(0),
          events(new io_event[max_events]),
          requests(new Request[max_events]),
          num_submitted(0)
    {
        for(size_t i=0; i < max_events; ++i) {
            requests_available.push(requests.get()+i);
        }

        constexpr int kPageSize = 4096;

        fd = open(filename.c_str(), O_RDWR | O_DIRECT | O_NONBLOCK | O_CREAT, 0644);
        PANGO_ENSURE(fd >= 0);

//        if(fallocate(fd, 0, 0, kPageSize * length_) < 0)
//        {
//            PANGO_ENSURE(false);
//        }

        std::memset(&ioctx_, 0, sizeof(io_context_t));
        int res = io_setup(max_events, &ioctx_);
        PANGO_ENSURE(res >= 0);
    }

    ~LinuxAsioFile()
    {
        std::unique_lock<std::mutex> lq(queue_mutex);

//        // Wait for flush
//        {
//            while(Reap(true) == max_events);
//            PANGO_ASSERT(!requests_available.empty());
//            Request* req = requests_available.back();
//            requests_available.pop();
//            struct iocb iocb;
//            memset(&iocb, 0, sizeof(iocb));
//            iocb.aio_fildes = fd;
//            iocb.aio_lio_opcode = IO_CMD_NOOP;
//            iocb.key =RWF_SYNC;
//            iocb.aio_reqprio = 0;

//            io_prep_fsync(&iocb, fd);
//            iocb.data = req;
//            req->id = num_submitted++;
//            struct iocb* iocbs = &iocb;
//            int res = io_submit(ioctx_, 1, &iocbs);
//            PANGO_ENSURE(res == 1, "%", res);
//        }

        // Wait for pending IO
        while(Reap(true) > 0);

        io_destroy(ioctx_);
    }

    void Put(const uint8_t* data, size_t size_bytes, ssize_t offset = -1 ) override
    {
        {
            std::unique_lock<std::mutex> lq(queue_mutex);
            if(offset == -1) offset = end_bytes;
            end_bytes += size_bytes;
        }
        pwrite(fd, data, size_bytes, offset);
    }

    void Get(uint8_t* dst, size_t size_bytes, size_t file_offset_bytes, GetPolicy policy = GetPolicy::Throw) override
    {
        auto data = GetMapped(size_bytes, file_offset_bytes, policy);
        std::memcpy(dst, data.get(), size_bytes);
    }

    void PutAsync(const std::shared_ptr<uint8_t>& data, size_t size_bytes, ssize_t offset = -1) override
    {
        Request* req = nullptr;

        {
            std::unique_lock<std::mutex> lq(queue_mutex);

            // wait until we are able to submit
//            while(Reap(true) == max_events);
            PANGO_ASSERT(!requests_available.empty());

            if(offset == -1) offset = end_bytes;
            end_bytes += size_bytes;
            req = requests_available.back();
            requests_available.pop();
        }

        struct iocb iocb;
        io_prep_pwrite(&iocb, fd, data.get(), size_bytes, offset);
        iocb.data = req;
        req->id = num_submitted++;
        req->start_bytes = offset;
        req->size_bytes = size_bytes;

        struct iocb* iocbs = &iocb;
        int res = io_submit(ioctx_, 1, &iocbs);
        PANGO_ENSURE(res == 1);
    }

    std::shared_ptr<uint8_t>
    GetMapped(size_t size_bytes, size_t offset_bytes, GetPolicy policy = GetPolicy::Throw)  override
    {
        if(!mmap || mmap->size() < size_bytes) {
            std::error_code err;

            size_t file_size = mio::detail::query_file_size(fd, err);
            if(err) throw std::runtime_error(err.message());

            if(offset_bytes + size_bytes > file_size) {
                // Can't map this right now
                if(policy == GetPolicy::Throw) {
                    throw std::runtime_error("Get() called out of allocated file size.");
                }else if(policy == GetPolicy::Wait) {
                    throw std::runtime_error("Unsupported");
                }else if(policy == GetPolicy::Grow){
                    throw std::runtime_error("Unsupported");
                }
            }

            // We either don't have a mapping yet or it is partially mapped only
            if(offset_bytes + size_bytes < bytes_written) {
                // Create a new mmap.
                // The old will be freed when it is no longer references
                mmap = std::make_shared<mio::ummap_sink>();
                mmap->map(fd, 0, mio::map_entire_file, err);
                if(err) throw std::runtime_error(err.message());
            }
        }

        uint8_t* user_ptr =  mmap->data() + offset_bytes;
        return std::shared_ptr<uint8_t>(mmap, user_ptr);
    }

private:
    struct Request
    {
        unsigned id;
        size_t start_bytes;
        size_t size_bytes;
    };

    // Process completed IO
    // NOTE: Requires that queue_mutex is held already
    // Return requests pending
    int Reap(bool only_return_when_submit_possible)
    {
        const int min_ev = (only_return_when_submit_possible && requests_available.empty()) ? 1 : 0;
        const int num_events = io_getevents(ioctx_, min_ev, max_events, events.get(), nullptr);
        PANGO_ENSURE(num_events >= 0);
        for(size_t i=0; i < num_events; ++i) {
            io_event& ev = events.get()[i];
            Request* req = static_cast<Request*>(ev.data);
            std::cout << FormatString("% complete (% bytes, % min_ev, % pending)", req->id, long(ev.res), min_ev, max_events - requests_available.size()) << std::endl;
            requests_available.push(req);
        }

        return max_events - requests_available.size();
    }

    void Truncate(size_t size_bytes, std::error_code& )
    {
        ftruncate(fd, size_bytes);
    }

    // linux aio stuff
    io_context_t ioctx_;

    std::unique_ptr<io_event[]> events;
    std::unique_ptr<Request[]> requests;
    std::queue<Request*> requests_available;

    unsigned num_submitted;
    std::string filename;

    std::mutex queue_mutex;

    size_t bytes_written;
    size_t bytes_queued;
    size_t end_bytes;

    std::queue<std::function<void(void)>> to_write;

    // Seperate file descriptor for memory mapped reading.
    mio::file_handle_type fd;
    std::shared_ptr<mio::ummap_sink> mmap;
};

}
