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

#ifndef PANGOLIN_FFMPEG_H
#define PANGOLIN_FFMPEG_H

#include "pangolin.h"
#include "video.h"

#ifdef HAVE_FFMPEG

extern "C"
{
#include <avcodec.h>
#include <avformat.h>
#include <swscale.h>
}

namespace pangolin
{

class FfmpegVideo : public VideoInterface
{
public:
    FfmpegVideo(const char* filename);
    ~FfmpegVideo();

    //! Implement VideoSource::Start()
    void Start();

    //! Implement VideoSource::Stop()
    void Stop();

    unsigned Width() const;

    unsigned Height() const;

    bool GrabNext( unsigned char* image, bool wait = true );

    bool GrabNewest( unsigned char* image, bool wait = true );

protected:
    AVFormatContext *pFormatCtx;
    int             videoStream;
    AVCodecContext  *pCodecCtx;
    AVCodec         *pCodec;
    AVFrame         *pFrame;
    AVFrame         *pFrameRGB;
    AVPacket        packet;
    int             numBytes;
    uint8_t         *buffer;
};

}

#endif //HAVE_FFMPEG

#endif //PANGOLIN_FFMPEG_H
