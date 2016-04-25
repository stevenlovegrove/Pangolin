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

#define DEBUGTHREAD

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
    // queue destructor will deallocate buffers.
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
    return in_prop ? in_prop->DeviceProperties() : device_properties;
}

const json::value& ThreadVideo::FrameProperties() const
{
    VideoPropertiesInterface* in_prop = dynamic_cast<VideoPropertiesInterface*>(videoin[0]);
    return in_prop ? in_prop->FrameProperties() : frame_properties;
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
    // if something in queue return it
    if(queue.AvailableFrames() > 0) {
        unsigned char* i = queue.getNext();
        std::memcpy(image, i, queue.BufferSizeBytes());
        queue.returnUsedBuffer(i);
        return true;
    } else {
        if(wait) {
            // Blocking on notification from grab thread.
            std::unique_lock<std::mutex> lk(cvMtx);
            if(cv.wait_for(lk, boostd::chrono::seconds(5)) == boostd::cv_status::timeout)
                throw std::runtime_error("Thread: GrabNext blocking read for frames reached timeout.");
            unsigned char* i = queue.getNext();
            std::memcpy(image, i, queue.BufferSizeBytes());
            queue.returnUsedBuffer(i);
            return true;
        } else {
            // No wait, simply return false.
            return false;
        }
    }
}

//! Implement VideoInput::GrabNewest()
bool ThreadVideo::GrabNewest( unsigned char* image, bool wait )
{
    // if > 1 in queue, drop all but last & return last
    // else if wait, mutex block on grab thread
    // else return false
    // if something in queue return it
    if(queue.AvailableFrames() > 0) {
        unsigned char* i = queue.getNewest();
        std::memcpy(image, i, queue.BufferSizeBytes());
        queue.returnUsedBuffer(i);
        return true;
    } else {
        if(wait) {
            // Blocking on notification from grab thread.
            std::unique_lock<std::mutex> lk(cvMtx);
            if(cv.wait_for(lk, boostd::chrono::seconds(5)) == boostd::cv_status::timeout)
                throw std::runtime_error("Thread: GrabNewest blocking read for frames reached timeout.");
            unsigned char* i = queue.getNext();
            std::memcpy(image, i, queue.BufferSizeBytes());
            queue.returnUsedBuffer(i);
            return true;
        } else {
            // No wait, simply return false.
            return false;
        }
    }
}

void ThreadVideo::operator()()
{
    DBGPRINT("GRABTHREAD: Started.")
    while(!quit_grab_thread) {
        unsigned char* i = queue.getFreeBuffer();
        // Blocking Grab next with wait = true
        if(videoin[0]->GrabNext(i,false)){
            queue.addValidBuffer(i);
            DBGPRINT("GRABTHREAD: got frame.")
            // Let listening thread know we goit a frame in case they are waiting
            cv.notify_all();
        }
        boostd::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    DBGPRINT("GRABTHREAD: Stopped.")

    return;
}

}

#undef TSTART
#undef TGRABANDPRINT
#undef DBGPRINT
