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

#include "video_recorder.h"

using namespace std;

namespace pangolin
{

VideoRecorder::VideoRecorder(
    const std::string& filename,
    int stream0_width, int stream0_height, std::string stream0_fmt,
    unsigned int buffer_size_bytes
    ) : frames(0), writer(filename, buffer_size_bytes)
{
    VideoStream strm0;
    strm0.name = "main";
    strm0.w = stream0_width;
    strm0.h = stream0_height;
    strm0.fmt = stream0_fmt;
    strm0.frame_size_bytes = stream0_width * stream0_height * 3 * sizeof(unsigned char);

    stream_info.push_back(strm0);
}

VideoRecorder::~VideoRecorder()
{
}

void VideoRecorder::WriteFileHeader()
{
    // For single stream data (without any meta info), based loosely on
    // http://www.cse.yorku.ca/~jgryn/research/pvnspecs.html

    // colour (6) unsigned (a)
    writer.file << "PV6a\n";
    writer.file << stream_info[0].w  << "\n";
    writer.file << stream_info[0].h  << "\n";

    // depth: 0 = streaming
    writer.file << "0\n";

    // bits per channel
    writer.file << 8 << "\n";

    writer.file << "30.0\n";
}

int VideoRecorder::RecordFrame(void* img)
{
    if( stream_info.size() != 1 )
        throw VideoRecorderException("Incorrect number of frames specified");

    if(frames==0)
        WriteFileHeader();

    const VideoStream& strm = stream_info[0];

    writer.write((char*)img,strm.frame_size_bytes);

//    file.write((char*)img,strm.frame_size_bytes);

    return frames++;
}

}
