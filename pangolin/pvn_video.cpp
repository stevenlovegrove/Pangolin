/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011 Steven Lovegrove
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

#include "pvn_video.h"

#include <iostream>

using namespace std;

namespace pangolin
{

PvnVideo::PvnVideo(const char* filename, bool realtime )
    : realtime(realtime), last_frame(TimeNow())
{
    file.open(filename, ios::binary );

    if(!file.is_open() )
        throw VideoException("Cannot open file - does not exist or bad permissions.");

    ReadFileHeader();
}

PvnVideo::~PvnVideo()
{

}

void PvnVideo::ReadFileHeader()
{
    string sfmt;
    float framerate;

    VideoStream strm0;
    file >> sfmt;
    file >> strm0.w;
    file >> strm0.h;
    file >> framerate;
    file.get();

    if(file.bad() || !(strm0.w >0 && strm0.h >0) )
        throw VideoException("Unable to read video header");

    strm0.fmt = VideoFormatFromString(sfmt);
    strm0.frame_size_bytes = (strm0.w * strm0.h * strm0.fmt.bpp) / 8;
    frame_interval = TimeFromSeconds( 1.0 / framerate);

    stream_info.push_back(strm0);
}


void PvnVideo::Start()
{
}

void PvnVideo::Stop()
{
}

unsigned PvnVideo::Width() const
{
    return stream_info[0].w;
}

unsigned PvnVideo::Height() const
{
    return stream_info[0].h;
}

size_t PvnVideo::SizeBytes() const
{
    return stream_info[0].frame_size_bytes;
}

std::string PvnVideo::PixFormat() const
{
    return stream_info[0].fmt.format;
}

bool PvnVideo::GrabNext( unsigned char* image, bool /*wait*/ )
{
    file.read((char*)image,stream_info[0].frame_size_bytes);

    const basetime next_frame = TimeAdd(last_frame, frame_interval);

    if( realtime ) {
        WaitUntil(next_frame);
    }

    last_frame = TimeNow();
    return file.good();
}

bool PvnVideo::GrabNewest( unsigned char* image, bool wait )
{
    return GrabNext(image,wait);
}

}
