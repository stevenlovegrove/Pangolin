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

// It is impossible to keep up with ffmpeg deprecations, so ignore these warnings.
#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wdeprecated"
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <array>
#include <pangolin/factory/factory_registry.h>
#include <pangolin/video/iostream_operators.h>
#include <pangolin/video/drivers/ffmpeg.h>
#include <pangolin/utils/file_extension.h>

extern "C"
{
#include <libavformat/avio.h>
#include <libavutil/mathematics.h>
#include <libavdevice/avdevice.h>
}

namespace pangolin
{

FfmpegVideo::FfmpegVideo(const std::string filename, const std::string strfmtout, const std::string codec_hint, bool dump_info, int user_video_stream, ImageDim size)
    :pFormatCtx(0)
{
    InitUrl(filename, strfmtout, codec_hint, dump_info, user_video_stream, size);
}

void FfmpegVideo::InitUrl(const std::string url, const std::string strfmtout, const std::string codec_hint, bool dump_info, int user_video_stream, ImageDim size)
{
    if( url.find('*') != url.npos )
        throw VideoException("Wildcards not supported. Please use ffmpegs printf style formatting for image sequences. e.g. img-000000%04d.ppm");

    // Register all formats and codecs
    av_register_all();
    // Register all devices
    avdevice_register_all();

    AVInputFormat* fmt = NULL;

    if( !codec_hint.empty() ) {
        fmt = av_find_input_format(codec_hint.c_str());
    }

#if (LIBAVFORMAT_VERSION_MAJOR >= 53)
    AVDictionary* options = nullptr;
    if(size.x != 0 && size.y != 0) {
        std::string s = std::to_string(size.x) + "x" + std::to_string(size.y);
        av_dict_set(&options, "video_size", s.c_str(), 0);
    }
    if( avformat_open_input(&pFormatCtx, url.c_str(), fmt, &options) )
#else
    // Deprecated - can't use with mjpeg
    if( av_open_input_file(&pFormatCtx, url.c_str(), fmt, 0, NULL) )
#endif
        throw VideoException("Couldn't open stream");

    if( !ToLowerCopy(codec_hint).compare("mjpeg") )
#ifdef HAVE_FFMPEG_MAX_ANALYZE_DURATION2
        pFormatCtx->max_analyze_duration2 = AV_TIME_BASE * 0.0;
#else
        pFormatCtx->max_analyze_duration = AV_TIME_BASE * 0.0;
#endif

    // Retrieve stream information
#if (LIBAVFORMAT_VERSION_MAJOR >= 53)
    if(avformat_find_stream_info(pFormatCtx, 0)<0)
#else
    // Deprecated
    if(av_find_stream_info(pFormatCtx)<0)
#endif
        throw VideoException("Couldn't find stream information");

    if(dump_info) {
        // Dump information about file onto standard error
#if (LIBAVFORMAT_VERSION_MAJOR >= 53)
        av_dump_format(pFormatCtx, 0, url.c_str(), false);
#else
        // Deprecated
        dump_format(pFormatCtx, 0, url.c_str(), false);
#endif
    }

    // Find the first video stream
    videoStream=-1;
    audioStream=-1;

    std::vector<int> videoStreams;
    std::vector<int> audioStreams;

    for(unsigned i=0; i<pFormatCtx->nb_streams; i++)
    {
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            videoStreams.push_back(i);
        }else if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO)
        {
            audioStreams.push_back(i);
        }
    }

    if(videoStreams.size()==0)
        throw VideoException("Couldn't find a video stream");

    if(0 <= user_video_stream && user_video_stream < (int)videoStreams.size() ) {
        videoStream = videoStreams[user_video_stream];
    }else{
        videoStream = videoStreams[0];
    }

    // Get a pointer to the codec context for the video stream
    pVidCodecCtx = pFormatCtx->streams[videoStream]->codec;

    // Find the decoder for the video stream
    pVidCodec=avcodec_find_decoder(pVidCodecCtx->codec_id);
    if(pVidCodec==0)
        throw VideoException("Codec not found");

    // Open video codec
#if LIBAVCODEC_VERSION_MAJOR > 52
    if(avcodec_open2(pVidCodecCtx, pVidCodec,0)<0)
#else
    if(avcodec_open(pVidCodecCtx, pVidCodec)<0)
#endif
        throw VideoException("Could not open codec");

    // Hack to correct wrong frame rates that seem to be generated by some codecs
    if(pVidCodecCtx->time_base.num>1000 && pVidCodecCtx->time_base.den==1)
        pVidCodecCtx->time_base.den=1000;


    // Allocate video frames
#if LIBAVUTIL_VERSION_MAJOR >= 54
    pFrame = av_frame_alloc();
    pFrameOut = av_frame_alloc();
#else
    // deprecated
    pFrame = avcodec_alloc_frame();
    pFrameOut = avcodec_alloc_frame();
#endif
    if(!pFrame || !pFrameOut)
        throw VideoException("Couldn't allocate frames");

    fmtout = FfmpegFmtFromString(strfmtout);
    if(fmtout == AV_PIX_FMT_NONE )
        throw VideoException("Output format not recognised",strfmtout);

    // Image dimensions
    const int w = pVidCodecCtx->width;
    const int h = pVidCodecCtx->height;

    // Determine required buffer size and allocate buffer
    numBytesOut=avpicture_get_size(fmtout, w, h);

    buffer= new uint8_t[numBytesOut];

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    avpicture_fill((AVPicture *)pFrameOut, buffer, fmtout, w, h);

    // Allocate SWS for converting pixel formats
    img_convert_ctx = sws_getContext(w, h,
                                     pVidCodecCtx->pix_fmt,
                                     w, h, fmtout, SWS_FAST_BILINEAR,
                                     NULL, NULL, NULL);
    if(img_convert_ctx == NULL) {
        throw VideoException("Cannot initialize the conversion context");
    }

    // Populate stream info for users to query
    const PixelFormat strm_fmt = PixelFormatFromString(FfmpegFmtToString(fmtout));
    const StreamInfo stream(strm_fmt, w, h, (w*strm_fmt.bpp)/8, 0);
    streams.push_back(stream);
}

FfmpegVideo::~FfmpegVideo()
{
    // Free the RGB image
    delete[] buffer;
    av_free(pFrameOut);

    // Free the YUV frame
    av_free(pFrame);

    // Close the codec
    avcodec_close(pVidCodecCtx);

    // Close the video file
#if (LIBAVFORMAT_VERSION_MAJOR >= 54 || (LIBAVFORMAT_VERSION_MAJOR >= 53 && LIBAVFORMAT_VERSION_MINOR >= 21) )
    avformat_close_input(&pFormatCtx);
#else
    // Deprecated
    av_close_input_file(pFormatCtx);
#endif

    // Free pixel conversion context
    sws_freeContext(img_convert_ctx);
}

const std::vector<StreamInfo>& FfmpegVideo::Streams() const
{
    return streams;
}

size_t FfmpegVideo::SizeBytes() const
{
    return numBytesOut;
}

void FfmpegVideo::Start()
{
}

void FfmpegVideo::Stop()
{
}

bool FfmpegVideo::GrabNext(unsigned char* image, bool /*wait*/)
{
    int gotFrame = 0;

    while(!gotFrame && av_read_frame(pFormatCtx, &packet)>=0)
    {
        // Is this a packet from the video stream?
        if(packet.stream_index==videoStream)
        {
            // Decode video frame
            avcodec_decode_video2(pVidCodecCtx, pFrame, &gotFrame, &packet);
        }

        // Did we get a video frame?
        if(gotFrame) {
            sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0, pVidCodecCtx->height, pFrameOut->data, pFrameOut->linesize);
            memcpy(image,pFrameOut->data[0],numBytesOut);
        }

        // Free the packet that was allocated by av_read_frame
        av_free_packet(&packet);
    }

    return gotFrame;
}

bool FfmpegVideo::GrabNewest(unsigned char *image, bool wait)
{
    return GrabNext(image,wait);
}

PANGOLIN_REGISTER_FACTORY(FfmpegVideo)
{
    struct FfmpegVideoFactory : public TypedFactoryInterface<VideoInterface> {
        std::map<std::string,Precedence> Schemes() const override
        {
            return {{"ffpeg",10}, {"file",15}, {"files",15}};
        }
        const char* Description() const override
        {
            return "Use the FFMPEG library to decode videos.";
        }
        ParamSet Params() const override
        {
            return {{
                {"fmt","RGB24","Use FFMPEG to decode to this output format."},
                {"stream","-1","Decode all streams (-1) or the selected stream only in a multi-stream video."},
                {"codec_hint","","Apply a hint to FFMPEG on codec. Examples include {MJPEG,video4linux,...}"},
                {"size","","Request a particular size output from FFMPEG"},
                {"verbose","0","Output FFMPEG instantiation information."},
            }};
        }
        std::unique_ptr<VideoInterface> Open(const Uri& uri) override {
            const std::array<std::string,43> ffmpeg_ext = {{
                ".3g2",".3gp", ".amv", ".asf", ".avi", ".drc", ".flv", ".f4v",
                ".f4p", ".f4a", ".f4b", ".gif", ".gifv", ".m4v", ".mkv", ".mng", ".mov", ".qt",
                ".mp4", ".m4p", ".m4v", ".mpg", ".mp2", ".mpeg", ".mpe", ".mpv", ".mpg", ".mpeg",
                ".m2v", ".mxf", ".nsv",  ".ogv", ".ogg", ".rm", ".rmvb", ".roq", ".svi", ".vob",
                ".webm", ".wmv", ".yuv", ".h264", ".h265"
            }};

            if(!uri.scheme.compare("file") || !uri.scheme.compare("files")) {
                const std::string ext = FileLowercaseExtention(uri.url);
                if(std::find(ffmpeg_ext.begin(), ffmpeg_ext.end(), ext) == ffmpeg_ext.end()) {
                    // Don't try to load unknown files without the ffmpeg:// scheme.
                    return std::unique_ptr<VideoInterface>();
                }
            }

            const bool verbose = uri.Get<bool>("verbose",false);
            std::string outfmt = uri.Get<std::string>("fmt","RGB24");
            std::string codec_hint = uri.Get<std::string>("codec_hint","");
            ToUpper(outfmt);
            ToUpper(codec_hint);
            const int video_stream = uri.Get<int>("stream",-1);
            const ImageDim size = uri.Get<ImageDim>("size",ImageDim(0,0));
            return std::unique_ptr<VideoInterface>( new FfmpegVideo(uri.url.c_str(), outfmt, codec_hint, verbose, video_stream) );
        }
    };

    return FactoryRegistry::I()->RegisterFactory<VideoInterface>(std::make_shared<FfmpegVideoFactory>());
}

}
