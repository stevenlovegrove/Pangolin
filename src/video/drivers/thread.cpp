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

namespace pangolin
{

ThreadVideo::ThreadVideo(VideoInterface* src): quit_grab_thread(false)
{
    if(!src) {
        throw VideoException("ThreadVideo: VideoInterface in must not be null");
    }
    videoin.push_back(src);
}

ThreadVideo::~ThreadVideo()
{
    Stop();
    delete videoin[0];
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
    return size_bytes;
}

//! Implement VideoInput::Streams()
const std::vector<StreamInfo>& ThreadVideo::Streams() const
{
    return streams;
}

std::vector<VideoInterface*>& ThreadVideo::InputStreams()
{
    return videoin;
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
    BufferAwareVideoInterface* vpi = dynamic_cast<BufferAwareVideoInterface*>(videoin[0]);
    if(!vpi)
    {
        return 0;
    }
    else
    {
        return vpi->AvailableFrames();
    }
}

bool ThreadVideo::DropNFrames(uint32_t n)
{
    BufferAwareVideoInterface* vpi = dynamic_cast<BufferAwareVideoInterface*>(videoin[0]);
    if(!vpi)
    {
        return false;
    }
    else
    {
        return vpi->DropNFrames(n);
    }
}

//! Implement VideoInput::GrabNext()
bool ThreadVideo::GrabNext( unsigned char* image, bool wait )
{
    if(videoin[0]->GrabNext(buffer,wait)) {
        return true;
    }else{
        return false;
    }
}

//! Implement VideoInput::GrabNewest()
bool ThreadVideo::GrabNewest( unsigned char* image, bool wait )
{
    if(videoin[0]->GrabNewest(buffer,wait)) {
        return true;
    }else{
        return false;
    }
}

void PleoraVideo::operator()()
{
    PvResult lResult;
    PvBuffer *lBuffer = NULL;
    PvResult lOperationResult;

    DBGPRINT("GRABTHREAD: Started.")
    while(!quit_grab_thread) {
        grabbedBuffListMtx.lock();
        // housekeeping
        GrabbedBufferList::iterator lIt =  lGrabbedBuffList.begin();
        while(lIt != lGrabbedBuffList.end()) {
            if(!lIt->valid) {
                lStreamMtx.lock();
                lStream->QueueBuffer(lIt->buff);
                lStreamMtx.unlock();
                lIt = lGrabbedBuffList.erase(lIt);
                DBGPRINT("GRABTHREAD: Requeued buffer.")
            } else {
                ++lIt;
            }
        }
        grabbedBuffListMtx.unlock();

        // Retrieve next buffer
        lStreamMtx.lock();
        lResult = lStream->RetrieveBuffer( &lBuffer, &lOperationResult, 50000);
        lStreamMtx.unlock();

        if ( !lResult.IsOK() ) {
            if(lResult && (lResult.GetCode() == PvResult::Code::NO_MORE_ITEM)) {
                // No more buffer left in the queue, wait a bit before retrying.
                boostd::this_thread::sleep_for(boostd::chrono::milliseconds(5));
            } else if(lResult && !(lResult.GetCode() == PvResult::Code::TIMEOUT)) {
                pango_print_warn("Pleora error: %s,\n", lResult.GetCodeString().GetAscii());
            }
        } else {
            grabbedBuffListMtx.lock();
            lGrabbedBuffList.push_back(GrabbedBuffer(lBuffer,lOperationResult,true));
            ++validGrabbedBuffers;
            grabbedBuffListMtx.unlock();
            cv.notify_all();
        }
        boostd::this_thread::yield();
    }

    grabbedBuffListMtx.lock();
    lStreamMtx.lock();
    // housekeeping
    GrabbedBufferList::iterator lIt =  lGrabbedBuffList.begin();
    while(lIt != lGrabbedBuffList.end()) {
        lStream->QueueBuffer(lIt->buff);
        lIt = lGrabbedBuffList.erase(lIt);
    }
    validGrabbedBuffers = 0;
    lStreamMtx.unlock();
    grabbedBuffListMtx.unlock();

    DBGPRINT("GRABTHREAD: Stopped.")

    return;
}

}
