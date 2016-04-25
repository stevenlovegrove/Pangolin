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

#ifndef PANGOLIN_VIDEO_THREAD_H
#define PANGOLIN_VIDEO_THREAD_H

#include <pangolin/pangolin.h>
#include <pangolin/video/video.h>

#include <pangolin/compat/thread.h>
#include <pangolin/compat/mutex.h>
#include <pangolin/compat/condition_variable.h>

namespace pangolin
{

class FixSizeBuffersQueue
{

public:
    typedef unsigned char* BufPType;

    FixSizeBuffersQueue() {}

    ~FixSizeBuffersQueue() {
        // Deallocate everything.
        std::lock_guard<std::mutex> vlock(vMtx);
        while(validBuffers.size() > 0){
            delete[] validBuffers.front();
            validBuffers.pop_front();
        }
        std::lock_guard<std::mutex> elock(eMtx);
        while(emptyBuffers.size() > 0){
            delete[] emptyBuffers.front();
            emptyBuffers.pop_front();
        }
    }

    void init(unsigned int num, unsigned int sizeBytes) {
        maxNumBuffers = num;
        bufferSizeBytes = sizeBytes;
        // lock queue
        std::lock_guard<std::mutex> vlock(vMtx);
        std::lock_guard<std::mutex> elock(eMtx);

        // Put back any valid buffer to the available buffers queue.
        while(validBuffers.size() > 0){
            emptyBuffers.push_back(validBuffers.front());
            validBuffers.pop_front();
        }
        // Allocate buffers
        while(emptyBuffers.size() < maxNumBuffers) {
            emptyBuffers.push_back(new unsigned char[bufferSizeBytes]);
        }
    }

    BufPType getNewest() {
        std::lock_guard<std::mutex> vlock(vMtx);
        std::lock_guard<std::mutex> elock(eMtx);
        if(validBuffers.size() == 0) {
            // Empty queue.
            return 0;
        } else {
            // Requeue all but newest buffers.
            while(validBuffers.size() > 1) {
                emptyBuffers.push_back(validBuffers.front());
                validBuffers.pop_front();
            }
            // Return newest buffer.
            BufPType bp = validBuffers.front();
            validBuffers.pop_front();
            return bp;
        }
    }

    BufPType getNext() {
        std::lock_guard<std::mutex> vlock(vMtx);
        if(validBuffers.size() == 0) {
            // Empty queue.
            return 0;
        } else {
            // Return oldest buffer.
            BufPType bp = validBuffers.front();
            validBuffers.pop_front();
            return bp;
        }
    }

    BufPType getFreeBuffer() {
        std::lock_guard<std::mutex> vlock(vMtx);
        std::lock_guard<std::mutex> elock(eMtx);
        if(emptyBuffers.size() > 0) {
            // Simply get a free buffer from the free buffers list.
            BufPType bp = emptyBuffers.front();
            emptyBuffers.pop_front();
            return bp;
        } else {
            if(validBuffers.size() == 0) {
                // Queue not yet initialized.
                return 0;
            } else {
                // No free buffers return oldest among the valid buffers.
                BufPType bp = validBuffers.front();
                validBuffers.pop_front();
                return bp;
            }
        }
    }

    void addValidBuffer(BufPType bp) {
        // Add buffer to valid buffers queue.
        std::lock_guard<std::mutex> vlock(vMtx);
        validBuffers.push_back(bp);
    }

    void returnUsedBuffer(BufPType bp) {
        // Add buffer back to empty buffers queue.
        std::lock_guard<std::mutex> elock(eMtx);
        emptyBuffers.push_back(bp);
    }

    const unsigned int AvailableFrames() const {
        std::lock_guard<std::mutex> vlock(vMtx);
        return validBuffers.size();
    }

    bool DropNFrames(unsigned int n) {
        std::lock_guard<std::mutex> vlock(vMtx);
        if(validBuffers.size() < n) {
            return false;
        } else {
            std::lock_guard<std::mutex> elock(eMtx);
            // Requeue all but newest buffers.
            for(unsigned int i=0; i<n; ++i) {
                emptyBuffers.push_back(validBuffers.front());
                validBuffers.pop_front();
            }
            return true;
        }
    }

    unsigned int BufferSizeBytes(){
        return bufferSizeBytes;
    }

private:
    std::list<BufPType> validBuffers;
    std::list<BufPType> emptyBuffers;
    mutable boostd::mutex vMtx;
    boostd::mutex eMtx;
    unsigned int maxNumBuffers;
    unsigned int bufferSizeBytes;
};

// Video class that creates a thread that keeps pulling frames and processing from its children.
class PANGOLIN_EXPORT ThreadVideo :
        public VideoInterface, public VideoPropertiesInterface, public BufferAwareVideoInterface
{
public:
    ThreadVideo(VideoInterface* videoin, unsigned int num_buffers);
    ~ThreadVideo();

    //! Implement VideoInput::Start()
    void Start();

    //! Implement VideoInput::Stop()
    void Stop();

    //! Implement VideoInput::SizeBytes()
    size_t SizeBytes() const;

    //! Implement VideoInput::Streams()
    const std::vector<StreamInfo>& Streams() const;

    //! Implement VideoInput::GrabNext()
    bool GrabNext( unsigned char* image, bool wait = true );

    //! Implement VideoInput::GrabNewest()
    bool GrabNewest( unsigned char* image, bool wait = true );

    const json::value& DeviceProperties() const;

    const json::value& FrameProperties() const;

    uint32_t AvailableFrames() const;

    bool DropNFrames(uint32_t n);

    void operator()();

protected:
    std::vector<VideoInterface*> videoin;

    bool quit_grab_thread;
    unsigned int num_buffers;
    FixSizeBuffersQueue queue;

    boostd::condition_variable cv;
    boostd::mutex cvMtx;
    boostd::thread grab_thread;

    json::value device_properties;
    json::value frame_properties;
};

}

#endif // PANGOLIN_VIDEO_THREAD_H
