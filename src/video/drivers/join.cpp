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

#include <pangolin/video/drivers/join.h>

namespace pangolin
{
VideoJoiner::VideoJoiner(const std::vector<VideoInterface*>& src)
    : src(src), size_bytes(0), sync_attempts_to_go(-1), sync_continuously(false)
{
    // Add individual streams
    for(size_t s=0; s< src.size(); ++s)
    {
        VideoInterface& vid = *src[s];
        for(size_t i=0; i < vid.Streams().size(); ++i)
        {
            const StreamInfo si = vid.Streams()[i];
            const VideoPixelFormat fmt = si.PixFormat();
            const Image<unsigned char> img_offset = si.StreamImage((unsigned char*)size_bytes);
            streams.push_back(StreamInfo(fmt, img_offset));
        }
        size_bytes += src[s]->SizeBytes();
    }
}

VideoJoiner::~VideoJoiner()
{
    for(size_t s=0; s< src.size(); ++s) {
        src[s]->Stop();
        delete src[s];
    }
}

size_t VideoJoiner::SizeBytes() const
{
    return size_bytes;
}

const std::vector<StreamInfo>& VideoJoiner::Streams() const
{
    return streams;
}

void VideoJoiner::Start()
{
    for(size_t s=0; s< src.size(); ++s) {
        src[s]->Start();
    }
}

void VideoJoiner::Stop()
{
    for(size_t s=0; s< src.size(); ++s) {
        src[s]->Stop();
    }
}

bool VideoJoiner::Sync(int64_t tol, bool continuous)
{
    for(size_t s=0; s< src.size(); ++s)
    {
       VideoPropertiesInterface* vpi = dynamic_cast<VideoPropertiesInterface*>(src[s]);
       if(!vpi)
         return false;
    }
    sync_attempts_to_go = MAX_SYNC_ATTEMPTS;
    sync_tolerance_us = tol;
    sync_continuously = continuous;
    return true;
}

bool VideoJoiner::GrabNext( unsigned char* image, bool wait )
{
    size_t offset = 0;
    std::vector<size_t> offsets;
    int64_t newest = std::numeric_limits<int64_t>::min();
    int64_t oldest = std::numeric_limits<int64_t>::max();
    bool grabbed_any = false;

    for(size_t s=0; s<src.size(); ++s) {
       VideoInterface& vid = *src[s];
       grabbed_any |= vid.GrabNext(image+offset,wait);
       offsets.push_back(offset);
       offset += vid.SizeBytes();
       if(sync_attempts_to_go >= 0) {
          VideoPropertiesInterface* vidpi = dynamic_cast<VideoPropertiesInterface*>(src[s]);
          if(vidpi->FrameProperties().contains(PANGO_HOST_RECEPTION_TIME_US)) {
             int64_t rt = vidpi->FrameProperties()[PANGO_HOST_RECEPTION_TIME_US].get<int64_t>();
             reception_times.push_back(rt);
             if(newest < rt) newest = rt;
             if(oldest > rt) oldest = rt;
          } else {
             sync_attempts_to_go = -1;
             pango_print_error("Stream %lu in join does not support startup_sync_us option.\n", s);
          }
       }
    }

    if((sync_continuously || (sync_attempts_to_go == 0)) && ((newest - oldest) > sync_tolerance_us) ){
       pango_print_warn("Join error, unable to sync streams within %lu us\n", sync_tolerance_us);
    }

    if(sync_attempts_to_go >= 0) {
       for(size_t s=0; s<src.size(); ++s) {
          if(reception_times[s] < (newest - sync_tolerance_us)) {
             VideoInterface& vid = *src[s];
             vid.GrabNewest(image+offsets[s],false);
          }
       }
       if(!sync_continuously) --sync_attempts_to_go;
    }

    return grabbed_any;
}

bool VideoJoiner::GrabNewest( unsigned char* image, bool wait )
{
    return GrabNext(image, wait);
}

std::vector<VideoInterface*>& VideoJoiner::InputStreams()
{
    return src;
}

}
