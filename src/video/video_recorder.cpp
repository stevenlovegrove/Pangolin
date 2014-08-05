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

#include <pangolin/video/video_recorder.h>

using namespace std;

namespace pangolin
{

VideoRecorder::VideoRecorder(
    const std::string& filename,
    int stream0_width, int stream0_height, std::string stream0_fmt,
    unsigned int buffer_size_bytes
    ) : frames(0), buffer(filename, buffer_size_bytes), writer(&buffer)
{
    const VideoPixelFormat fmt = VideoFormatFromString(stream0_fmt);
    const StreamInfo strm0(fmt, stream0_width, stream0_height, (stream0_width*fmt.bpp)/8, 0);
    streams.push_back(strm0);
}

VideoRecorder::~VideoRecorder()
{
}

void VideoRecorder::WriteFileHeader()
{
    writer << streams[0].PixFormat().format << "\n";
    writer << streams[0].Width()  << "\n";
    writer << streams[0].Height()  << "\n";
    writer << "30.0\n";
}

int VideoRecorder::RecordFrame(uint8_t* img)
{
    if( streams.size() != 1 )
        throw VideoRecorderException("Incorrect number of frames specified");

    if(frames==0)
        WriteFileHeader();

    writer.write((char*)img, streams[0].SizeBytes() );

    return frames++;
}

}
