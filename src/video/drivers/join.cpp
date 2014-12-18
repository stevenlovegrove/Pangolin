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
    : src(src), size_bytes(0)
{
    // Add individual streams
    for(int s=0; s< src.size(); ++s)
    {
        VideoInterface& vid = *src[s];
        for(int i=0; i < vid.Streams().size(); ++i)
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
    for(int s=0; s< src.size(); ++s) {
        src[s]->Start();
    }
}

void VideoJoiner::Stop()
{
    for(int s=0; s< src.size(); ++s) {
        src[s]->Stop();
    }
}

bool VideoJoiner::GrabNext( unsigned char* image, bool wait )
{
    bool grabbed_any = false;
    size_t offset = 0;
    for(int s=0; s< src.size(); ++s)
    {
        VideoInterface& vid = *src[s];
        grabbed_any |= vid.GrabNext(image+offset,wait);
        offset += vid.SizeBytes();
    }
    return grabbed_any;
}

bool VideoJoiner::GrabNewest( unsigned char* image, bool wait )
{
    return GrabNext(image, wait);
}

std::vector<VideoInterface*>& VideoJoiner::SourceVideos()
{
    return src;
}

}
