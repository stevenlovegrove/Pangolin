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

// Video class that creates a thread that keeps pulling frames and processing from its children.
class PANGOLIN_EXPORT ThreadVideo :
        public VideoInterface, public VideoPropertiesInterface, public BufferAwareVideoInterface
{
public:
    ThreadVideo(VideoInterface* videoin);
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

    std::vector<VideoInterface*>& InputStreams();

    const json::value& DeviceProperties() const {
        return device_properties;
    }

    const json::value& FrameProperties() const {
        return frame_properties;
    }

    uint32_t AvailableFrames() const;

    bool DropNFrames(uint32_t n);

    void operator()();
protected:
    std::vector<VideoInterface*> videoin;
    std::vector<StreamInfo> streams;

    bool quit_grab_thread;
    boostd::mutex lStreamMtx;
    boostd::mutex grabbedBuffListMtx;
    boostd::condition_variable cv;
    boostd::thread grab_thread;
    std::mutex cv_m;

    json::value device_properties;
    json::value frame_properties;
};

}

#endif // PANGOLIN_VIDEO_THREAD_H
