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

#pragma once

#include <pangolin/video/video_interface.h>
#include <pangolin/video/iostream_operators.h>
#include <pangolin/video/drivers/ffmpeg_common.h>

namespace pangolin
{

class PANGOLIN_EXPORT FfmpegVideo : public VideoInterface, public VideoPlaybackInterface
{
public:
    FfmpegVideo(const std::string filename, const std::string fmtout = "RGB24", const std::string codec_hint = "", bool dump_info = false, int user_video_stream = -1, ImageDim size = ImageDim(0,0));
    ~FfmpegVideo();
    
    //! Implement VideoInput::Start()
    void Start() override;
    
    //! Implement VideoInput::Stop()
    void Stop() override;

    //! Implement VideoInput::SizeBytes()
    size_t SizeBytes() const override;

    //! Implement VideoInput::Streams()
    const std::vector<StreamInfo>& Streams() const override;
    
    //! Implement VideoInput::GrabNext()
    bool GrabNext( unsigned char* image, bool wait = true ) override;
    
    //! Implement VideoInput::GrabNewest()
    bool GrabNewest( unsigned char* image, bool wait = true ) override;
    
    //! VideoPlaybackInterface methods
    size_t GetCurrentFrameId() const override;
    size_t GetTotalFrames() const override;
    size_t Seek(size_t frameid) override;

protected:
    void InitUrl(const std::string filename, const std::string fmtout = "RGB24", const std::string codec_hint = "", bool dump_info = false , int user_video_stream = -1, ImageDim size= ImageDim(0,0));
    
    std::vector<StreamInfo> streams;
    
    SwsContext      *img_convert_ctx;
    AVFormatContext *pFormatCtx;
    int             videoStream;
    int64_t         numFrames;
    int64_t         ptsPerFrame;
    const AVCodec         *pVidCodec;
    const AVCodec         *pAudCodec;
    AVCodecContext *pCodecContext;
    AVFrame         *pFrame;
    AVFrame         *pFrameOut;
    AVPacket        *packet;
    int             numBytesOut;
    AVPixelFormat     fmtout;
    size_t next_frame;
};

}
