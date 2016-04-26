/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2014 Steven Lovegrove
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <pangolin/video/drivers/thread.h>

#ifdef DEBUGTHREAD
  #include <pangolin/utils/timer.h>
  #define TSTART() pangolin::basetime start,last,now; start = pangolin::TimeNow(); last = start;
  #define TGRABANDPRINT(...)  now = pangolin::TimeNow(); fprintf(stderr,"THREAD: "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, " %fms.\n",1000*pangolin::TimeDiff_s(last, now)); last = now;
  #define DBGPRINT(...) fprintf(stderr,"THREAD: "); fprintf(stderr, __VA_ARGS__); fprintf(stderr,"\n");
#else
  #define TSTART()
  #define TGRABANDPRINT(...)
  #define DBGPRINT(...)
#endif

namespace pangolin
{

ThreadVideo::ThreadVideo(VideoInterface* src, unsigned int num_buffers): quit_grab_thread(true), num_buffers(num_buffers)
{
    if(!src) {
        throw VideoException("ThreadVideo: VideoInterface in must not be null");
    }
    videoin.push_back(src);
    // queue init allocates buffers.
    queue.init(num_buffers, videoin[0]->SizeBytes());
    Start();
}

ThreadVideo::~ThreadVideo()
{
    Stop();
    delete videoin[0];
    // queue destructor will take care of deallocating buffers.
}

//! Implement VideoInput::Start()
void ThreadVideo::Start()
{
    videoin[0]->Start();
    quit_grab_thread = false;
    grab_thread = boostd::thread(boostd::ref(*this));
}

//! Implement VideoInput::Stop()
void ThreadVideo::Stop()
{
    quit_grab_thread = true;
    if(grab_thread.joinable()) {
        grab_thread.join();
    }
    videoin[0]->Stop();
}

//! Implement VideoInput::SizeBytes()
size_t ThreadVideo::SizeBytes() const
{
    return videoin[0]->SizeBytes();
}

//! Implement VideoInput::Streams()
const std::vector<StreamInfo>& ThreadVideo::Streams() const
{
    return videoin[0]->Streams();
}

const json::value& ThreadVideo::DeviceProperties() const
{
    VideoPropertiesInterface* in_prop = dynamic_cast<VideoPropertiesInterface*>(videoin[0]);
    if(!in_prop)
        throw std::runtime_error("ThreadVideo: video in interface does not implement VideoPropertiesInterface.");
    else
        return in_prop->FrameProperties();
}

const json::value& ThreadVideo::FrameProperties() const
{
    VideoPropertiesInterface* in_prop = dynamic_cast<VideoPropertiesInterface*>(videoin[0]);
    if(!in_prop)
        throw std::runtime_error("ThreadVideo: video in interface does not implement VideoPropertiesInterface.");
    else
        return in_prop->FrameProperties();
}

unsigned int ThreadVideo::AvailableFrames() const
{
    return queue.AvailableFrames();
}

bool ThreadVideo::DropNFrames(uint32_t n)
{
    return queue.DropNFrames(n);
}

//! Implement VideoInput::GrabNext()
bool ThreadVideo::GrabNext( unsigned char* image, bool wait )
{
    TSTART()
    if(queue.AvailableFrames() > 0) {
        // At least one valid frame in queue, return it.
        DBGPRINT("GrabNext at least one frame available.");
        unsigned char* i = queue.getNext();
        std::memcpy(image, i, queue.BufferSizeBytes());
        queue.returnUsedBuffer(i);
        TGRABANDPRINT("GrabNext memcpy of available frame took")
        return true;
    } else {
        if(wait) {
            // Must return a frame so block on notification from grab thread.
            std::unique_lock<boostd::mutex> lk(cvMtx);
            DBGPRINT("GrabNext no available frames wait for notification.");
            if(cv.wait_for(lk, boostd::chrono::seconds(5)) == boostd::cv_status::timeout)
                throw std::runtime_error("ThreadVideo: GrabNext blocking read for frames reached timeout.");
            DBGPRINT("GrabNext got notification.");
            unsigned char* i = queue.getNext();
            std::memcpy(image, i, queue.BufferSizeBytes());
            queue.returnUsedBuffer(i);
            TGRABANDPRINT("GrabNext wait for lock and memcpy of available frame took")
            return true;
        } else {
            DBGPRINT("GrabNext no available frames no wait.");
            // No frames available, no wait, simply return false.
            return false;
        }
    }
}

//! Implement VideoInput::GrabNewest()
bool ThreadVideo::GrabNewest( unsigned char* image, bool wait )
{
    TSTART()
    if(queue.AvailableFrames() > 0) {
        // At least one valid frame in queue, call grabNewest to drop old frame and return the newest.
        DBGPRINT("GrabNewest at least one frame available.");
        unsigned char* i = queue.getNewest();
        std::memcpy(image, i, queue.BufferSizeBytes());
        queue.returnUsedBuffer(i);
        TGRABANDPRINT("GrabNewest memcpy of available frame took")
        return true;
    } else {
        if(wait) {
            // Must return a frame so block on notification from grab thread.
            DBGPRINT("GrabNewest no available frames wait for notification.");
            boostd::unique_lock<boostd::mutex> lk(cvMtx);
            if(cv.wait_for(lk, boostd::chrono::seconds(5)) == boostd::cv_status::timeout)
                throw std::runtime_error("ThreadVideo: GrabNewest blocking read for frames reached timeout.");
            DBGPRINT("GrabNewest got notification.");
            unsigned char* i = queue.getNext();
            std::memcpy(image, i, queue.BufferSizeBytes());
            queue.returnUsedBuffer(i);
            TGRABANDPRINT("GrabNwest wait for lock and memcpy of available frame took")
            return true;
        } else {
            DBGPRINT("GrabNewest no available frames no wait.");
            // No frames available, no wait, simply return false.
            return false;
        }
    }
}

void ThreadVideo::operator()()
{
    DBGPRINT("Grab thread Started.")
    // Spinning thread attempting to read from videoin[0] as fast as possible
    // relying on the videoin[0] blocking grab.
    while(!quit_grab_thread) {
        // Get a buffer from the queue;
        unsigned char* i = queue.getFreeBuffer();
        // Blocking grab (i.e. GrabNext with wait = true).
        if(videoin[0]->GrabNext(i, true)){
            queue.addValidBuffer(i);
            DBGPRINT("Grab thread got frame. valid:%d free:%d",queue.AvailableFrames(),queue.EmptyBuffers())
            // Let listening threads know we got a frame in case they are waiting.
            cv.notify_all();
        } else {
            // Grabbing frames failed, requeue the buffer.
            queue.returnUsedBuffer(i);
        }
        boostd::this_thread::yield();
    }
    DBGPRINT("Grab thread Stopped.")

    return;
}

}

#undef TSTART
#undef TGRABANDPRINT
#undef DBGPRINT
