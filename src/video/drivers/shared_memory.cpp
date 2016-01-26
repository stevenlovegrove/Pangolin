#include <pangolin/video/drivers/shared_memory.h>

using namespace std;

namespace pangolin
{

SharedMemoryVideo::SharedMemoryVideo(size_t w, size_t h, std::string pix_fmt,
    const boostd::shared_ptr<SharedMemoryBufferInterface>& shared_memory,
    const boostd::shared_ptr<ConditionVariableInterface>& buffer_full) :
    _fmt(VideoFormatFromString(pix_fmt)),
    _w(w),
    _h(h),
    _frame_size(w*h*_fmt.bpp/8),
    _shared_memory(shared_memory),
    _buffer_full(buffer_full)
{
    const StreamInfo stream(_fmt, w, h, _frame_size, 0);
    _streams.push_back(stream);
}

SharedMemoryVideo::~SharedMemoryVideo()
{
}

void SharedMemoryVideo::Start()
{
}

void SharedMemoryVideo::Stop()
{
}

size_t SharedMemoryVideo::SizeBytes() const
{
    return _frame_size;
}

const std::vector<StreamInfo>& SharedMemoryVideo::Streams() const
{
    return _streams;
}

bool SharedMemoryVideo::GrabNext(unsigned char* image, bool wait)
{
    if (wait) {
        _buffer_full->wait();
    } else if (!_buffer_full->wait(TimeNow())) {
        return false;
    }

    _shared_memory->lock();
    memcpy(image, _shared_memory->ptr(), _frame_size);
    _shared_memory->unlock();

    return true;
}

bool SharedMemoryVideo::GrabNewest(unsigned char* image, bool wait)
{
    return GrabNext(image,wait);
}

}
