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
#include <pangolin/video/drivers/ffmpeg_common.h>

namespace pangolin {

enum FfmpegMethod
{
    FFMPEG_FAST_BILINEAR =    1,
    FFMPEG_BILINEAR      =    2,
    FFMPEG_BICUBIC       =    4,
    FFMPEG_X             =    8,
    FFMPEG_POINT         = 0x10,
    FFMPEG_AREA          = 0x20,
    FFMPEG_BICUBLIN      = 0x40,
    FFMPEG_GAUSS         = 0x80,
    FFMPEG_SINC          =0x100,
    FFMPEG_LANCZOS       =0x200,
    FFMPEG_SPLINE        =0x400
};

class PANGOLIN_EXPORT FfmpegConverter : public VideoInterface
{
public:
    FfmpegConverter(std::unique_ptr<VideoInterface>& videoin, const std::string pixelfmtout = "RGB24", FfmpegMethod method = FFMPEG_POINT);
    ~FfmpegConverter();

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

protected:
    std::vector<StreamInfo> streams;

    struct ConvertContext
    {
        SwsContext*     img_convert_ctx;
        AVPixelFormat   fmtsrc;
        AVPixelFormat   fmtdst;
        AVFrame*        avsrc;
        AVFrame*        avdst;
        size_t          w,h;
        size_t          src_buffer_offset;
        size_t          dst_buffer_offset;

        void convert(const unsigned char * src, unsigned char* dst);

    };

    std::unique_ptr<VideoInterface> videoin;
    std::unique_ptr<unsigned char[]> input_buffer;

    std::vector<ConvertContext> converters;
    //size_t src_buffer_size;
    size_t dst_buffer_size;
};

}
