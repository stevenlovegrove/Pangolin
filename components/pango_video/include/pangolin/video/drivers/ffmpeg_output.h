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

#include <pangolin/video/video_output_interface.h>
#include <pangolin/video/drivers/ffmpeg_common.h>

namespace pangolin
{

#if (LIBAVFORMAT_VERSION_MAJOR > 55) || ((LIBAVFORMAT_VERSION_MAJOR == 55) && (LIBAVFORMAT_VERSION_MINOR >= 7))
typedef AVCodecID CodecID;
#endif

// Forward declaration
class FfmpegVideoOutputStream;

class PANGOLIN_EXPORT FfmpegVideoOutput
    : public VideoOutputInterface
{
    friend class FfmpegVideoOutputStream;
public:
    FfmpegVideoOutput( const std::string& filename, int base_frame_rate, int bit_rate, bool flip = false);
    ~FfmpegVideoOutput();

    const std::vector<StreamInfo>& Streams() const override;

    void SetStreams(const std::vector<StreamInfo>& streams, const std::string& uri, const picojson::value& properties) override;

    int WriteStreams(const unsigned char* data, const picojson::value& frame_properties) override;

    bool IsPipe() const override;

protected:
    void Initialise(std::string filename);
    void StartStream();
    void Close();

    std::string filename;
    bool started;
    AVFormatContext *oc;
    std::vector<FfmpegVideoOutputStream*> streams;
    std::vector<StreamInfo> strs;

    int frame_count;

    int base_frame_rate;
    int bit_rate;
    bool is_pipe;
    bool flip;
};

class FfmpegVideoOutputStream
{
public:
    FfmpegVideoOutputStream(FfmpegVideoOutput& recorder, CodecID codec_id, uint64_t frame_rate, int bit_rate, const StreamInfo& input_info, bool flip );
    ~FfmpegVideoOutputStream();

    const StreamInfo& GetStreamInfo() const;

    void WriteImage(const uint8_t* img, int w, int h);
    void Flush();

protected:
    void WriteAvPacket(AVPacket* pkt);
    void WriteFrame(AVFrame* frame);
    double BaseFrameTime();

    FfmpegVideoOutput& recorder;

    StreamInfo input_info;
    AVPixelFormat input_format;
    AVPixelFormat output_format;
    int64_t last_pts;

    // These pointers are owned by class
    AVStream* stream;
    SwsContext *sws_ctx;
    AVFrame* src_frame;
    AVFrame* frame;
    AVCodecContext* codec_context;

    bool flip;
};

}
