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

#include <pangolin/pangolin.h>
#include <pangolin/video.h>

extern "C"
{

// HACK for some versions of FFMPEG
#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>
#include <libavcodec/avcodec.h>
}

namespace pangolin
{

class FfmpegVideo : public VideoInterface
{
public:
    FfmpegVideo(const std::string filename, const std::string fmtout = "RGB24", const std::string codec_hint = "", bool dump_info = false, int user_video_stream = -1);
    ~FfmpegVideo();
    
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
    void InitUrl(const std::string filename, const std::string fmtout = "RGB24", const std::string codec_hint = "", bool dump_info = false , int user_video_stream = -1);
    
    std::vector<StreamInfo> streams;
    
    SwsContext      *img_convert_ctx;
    AVFormatContext *pFormatCtx;
    int             videoStream;
    int             audioStream;
    AVCodecContext  *pVidCodecCtx;
    AVCodecContext  *pAudCodecCtx;
    AVCodec         *pVidCodec;
    AVCodec         *pAudCodec;
    AVFrame         *pFrame;
    AVFrame         *pFrameOut;
    AVPacket        packet;
    int             numBytesOut;
    uint8_t         *buffer;
    PixelFormat     fmtout;
};

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

class FfmpegConverter : public VideoInterface
{
public:
    FfmpegConverter(VideoInterface* videoin, const std::string pixelfmtout = "RGB24", FfmpegMethod method = FFMPEG_POINT);
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
    
    VideoInterface* videoin;
    SwsContext *img_convert_ctx;
    
    PixelFormat     fmtsrc;
    PixelFormat     fmtdst;
    AVFrame*        avsrc;
    AVFrame*        avdst;
    uint8_t*        bufsrc;
    uint8_t*        bufdst;
    int             numbytessrc;
    int             numbytesdst;
    unsigned        w,h;
};

#if (LIBAVFORMAT_VERSION_MAJOR > 55) || ((LIBAVFORMAT_VERSION_MAJOR == 55) && (LIBAVFORMAT_VERSION_MINOR >= 7))
typedef AVCodecID CodecID;
#endif

class FfmpegVideoOutput;
class FfmpegVideoOutputStream
    : public VideoOutputStreamInterface
{
public:
    FfmpegVideoOutputStream(FfmpegVideoOutput& recorder, CodecID codec_id, uint64_t frame_rate, int bit_rate, PixelFormat EncoderFormat, int width, int height );
    ~FfmpegVideoOutputStream();
    
    void WriteAvPacket(AVPacket* pkt);
    void WriteFrame(AVFrame* frame);
    void WriteImage(AVPicture& src_picture, int w, int h, PixelFormat fmt, int64_t pts);
    void WriteImage(uint8_t* img, int w, int h, const std::string& input_format, int64_t pts);
    void WriteImage(uint8_t* img, int w, int h, const std::string& input_format, double time);
    
    double BaseFrameTime();
    
protected:
    FfmpegVideoOutput& recorder;
    AVPicture dst_picture;
    int64_t last_pts;
    
    // These pointers are owned by class
    AVStream* stream;
    SwsContext *sws_ctx;
};

class FfmpegVideoOutput
    : public VideoOutputInterface
{
    friend class FfmpegVideoOutputStream;
public:
    FfmpegVideoOutput( const std::string& filename, int base_frame_rate, int bit_rate );
    ~FfmpegVideoOutput();
    
    void AddStream(int w, int h, const std::string& encoder_fmt);
    VideoOutputStreamInterface& operator[](size_t i);    
    
//    // Save img (with correct format and resolution) to video, returning video frame id.
//    int RecordFrame(uint8_t* img);
    
protected:
    void Initialise(std::string filename);
    void StartStream();
    void Close();
    
    std::string filename;
    bool started;
    AVFormatContext *oc;
    std::vector<FfmpegVideoOutputStream*> streams;
    
    int frame_count;
    
    int base_frame_rate;
    int bit_rate;    
};

}

#endif //PANGOLIN_FFMPEG_H
