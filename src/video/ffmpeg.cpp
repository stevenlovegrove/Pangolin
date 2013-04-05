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

#include <pangolin/video/ffmpeg.h>

extern "C"
{
#include <libavformat/avio.h>
#include <libavutil/mathematics.h>
}

namespace pangolin
{

PixelFormat FfmpegFmtFromString(const std::string fmt)
{
    std::string lfmt = boost::algorithm::to_lower_copy(fmt);
    if(!lfmt.compare("gray8") || !lfmt.compare("grey8") || !lfmt.compare("grey")) {
        return PIX_FMT_GRAY8;
    }
    return av_get_pix_fmt(lfmt.c_str());
}

#define TEST_PIX_FMT_RETURN(fmt) case PIX_FMT_##fmt: return #fmt;

std::string FfmpegFmtToString(const PixelFormat fmt)
{
    switch( fmt )
    {
    TEST_PIX_FMT_RETURN(YUV420P);
    TEST_PIX_FMT_RETURN(YUYV422);
    TEST_PIX_FMT_RETURN(RGB24);
    TEST_PIX_FMT_RETURN(BGR24);
    TEST_PIX_FMT_RETURN(YUV422P);
    TEST_PIX_FMT_RETURN(YUV444P);
    TEST_PIX_FMT_RETURN(YUV410P);
    TEST_PIX_FMT_RETURN(YUV411P);
    TEST_PIX_FMT_RETURN(GRAY8);
    TEST_PIX_FMT_RETURN(MONOWHITE);
    TEST_PIX_FMT_RETURN(MONOBLACK);
    TEST_PIX_FMT_RETURN(PAL8);
    TEST_PIX_FMT_RETURN(YUVJ420P);
    TEST_PIX_FMT_RETURN(YUVJ422P);
    TEST_PIX_FMT_RETURN(YUVJ444P);
    TEST_PIX_FMT_RETURN(XVMC_MPEG2_MC);
    TEST_PIX_FMT_RETURN(XVMC_MPEG2_IDCT);
    TEST_PIX_FMT_RETURN(UYVY422);
    TEST_PIX_FMT_RETURN(UYYVYY411);
    TEST_PIX_FMT_RETURN(BGR8);
    TEST_PIX_FMT_RETURN(BGR4);
    TEST_PIX_FMT_RETURN(BGR4_BYTE);
    TEST_PIX_FMT_RETURN(RGB8);
    TEST_PIX_FMT_RETURN(RGB4);
    TEST_PIX_FMT_RETURN(RGB4_BYTE);
    TEST_PIX_FMT_RETURN(NV12);
    TEST_PIX_FMT_RETURN(NV21);
    TEST_PIX_FMT_RETURN(ARGB);
    TEST_PIX_FMT_RETURN(RGBA);
    TEST_PIX_FMT_RETURN(ABGR);
    TEST_PIX_FMT_RETURN(BGRA);
    TEST_PIX_FMT_RETURN(GRAY16BE);
    TEST_PIX_FMT_RETURN(GRAY16LE);
    TEST_PIX_FMT_RETURN(YUV440P);
    TEST_PIX_FMT_RETURN(YUVJ440P);
    TEST_PIX_FMT_RETURN(YUVA420P);
    TEST_PIX_FMT_RETURN(VDPAU_H264);
    TEST_PIX_FMT_RETURN(VDPAU_MPEG1);
    TEST_PIX_FMT_RETURN(VDPAU_MPEG2);
    TEST_PIX_FMT_RETURN(VDPAU_WMV3);
    TEST_PIX_FMT_RETURN(VDPAU_VC1);
    TEST_PIX_FMT_RETURN(RGB48BE );
    TEST_PIX_FMT_RETURN(RGB48LE );
    TEST_PIX_FMT_RETURN(RGB565BE);
    TEST_PIX_FMT_RETURN(RGB565LE);
    TEST_PIX_FMT_RETURN(RGB555BE);
    TEST_PIX_FMT_RETURN(RGB555LE);
    TEST_PIX_FMT_RETURN(BGR565BE);
    TEST_PIX_FMT_RETURN(BGR565LE);
    TEST_PIX_FMT_RETURN(BGR555BE);
    TEST_PIX_FMT_RETURN(BGR555LE);
    TEST_PIX_FMT_RETURN(VAAPI_MOCO);
    TEST_PIX_FMT_RETURN(VAAPI_IDCT);
    TEST_PIX_FMT_RETURN(VAAPI_VLD);
    TEST_PIX_FMT_RETURN(YUV420P16LE);
    TEST_PIX_FMT_RETURN(YUV420P16BE);
    TEST_PIX_FMT_RETURN(YUV422P16LE);
    TEST_PIX_FMT_RETURN(YUV422P16BE);
    TEST_PIX_FMT_RETURN(YUV444P16LE);
    TEST_PIX_FMT_RETURN(YUV444P16BE);
    TEST_PIX_FMT_RETURN(VDPAU_MPEG4);
    TEST_PIX_FMT_RETURN(DXVA2_VLD);
    TEST_PIX_FMT_RETURN(RGB444BE);
    TEST_PIX_FMT_RETURN(RGB444LE);
    TEST_PIX_FMT_RETURN(BGR444BE);
    TEST_PIX_FMT_RETURN(BGR444LE);
    TEST_PIX_FMT_RETURN(Y400A   );
    TEST_PIX_FMT_RETURN(NB      );
    default: return "";
    }
}

#undef TEST_PIX_FMT_RETURN

FfmpegVideo::FfmpegVideo(const std::string filename, const std::string strfmtout, const std::string codec_hint, bool dump_info, int user_video_stream)
    :pFormatCtx(0)
{
    InitUrl(filename, strfmtout, codec_hint, dump_info, user_video_stream);
}

void FfmpegVideo::InitUrl(const std::string url, const std::string strfmtout, const std::string codec_hint, bool dump_info, int user_video_stream)
{
    if( url.find('*') != url.npos )
        throw VideoException("Wildcards not supported. Please use ffmpegs printf style formatting for image sequences. e.g. img-000000%04d.ppm");
    
    // Register all formats and codecs
    av_register_all();
    
    AVInputFormat* fmt = NULL;
    
    if( !codec_hint.empty() ) {
        fmt = av_find_input_format(codec_hint.c_str());
    }
    
#ifdef CODEC_TYPE_VIDEO
    // Old (deprecated) interface - can't use with mjpeg
    if( av_open_input_file(&pFormatCtx, url.c_str(), fmt, 0, NULL) )
#else
    if( avformat_open_input(&pFormatCtx, url.c_str(), fmt, NULL) )
#endif
        throw VideoException("Couldn't open stream");
    
    if( !boost::algorithm::to_lower_copy(codec_hint).compare("mjpeg") )
        pFormatCtx->max_analyze_duration = AV_TIME_BASE * 0.0;
    
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
#ifdef CODEC_TYPE_VIDEO
        // Old (deprecated) interface
        dump_format(pFormatCtx, 0, url.c_str(), false);
#else
        av_dump_format(pFormatCtx, 0, url.c_str(), false);
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
    
    // Allocate video frame
    pFrame=avcodec_alloc_frame();
    
    // Allocate an AVFrame structure
    pFrameOut=avcodec_alloc_frame();
    if(pFrameOut==0)
        throw VideoException("Couldn't allocate frame");
    
    fmtout = FfmpegFmtFromString(strfmtout);
    if(fmtout == PIX_FMT_NONE )
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
                                     w, h, fmtout, FFMPEG_POINT,
                                     NULL, NULL, NULL);
    if(img_convert_ctx == NULL) {
        throw VideoException("Cannot initialize the conversion context");
    }
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
#if (LIBAVFORMAT_VERSION_MAJOR >= 53)
    avformat_close_input(&pFormatCtx);
#else
    // Deprecated
    av_close_input_file(pFormatCtx);
#endif
    
    // Free pixel conversion context
    sws_freeContext(img_convert_ctx);
}


unsigned FfmpegVideo::Width() const
{
    return pVidCodecCtx->width;
}

unsigned FfmpegVideo::Height() const
{
    return pVidCodecCtx->height;
}

size_t FfmpegVideo::SizeBytes() const
{
    return numBytesOut;
}

VideoPixelFormat FfmpegVideo::PixFormat() const
{
    return VideoFormatFromString(FfmpegFmtToString(fmtout));
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

FfmpegConverter::FfmpegConverter(VideoInterface* videoin, const std::string pixelfmtout, FfmpegMethod method )
    :videoin(videoin)
{
    if( !videoin )
        throw VideoException("Source video interface not specified");
    
    w = videoin->Width();
    h = videoin->Height();
    fmtsrc = FfmpegFmtFromString(videoin->PixFormat());
    fmtdst = FfmpegFmtFromString(pixelfmtout);
    
    img_convert_ctx = sws_getContext(
                w, h, fmtsrc,
                w, h, fmtdst,
                method, NULL, NULL, NULL
                );
    if(!img_convert_ctx)
        throw VideoException("Could not create SwScale context for pixel conversion");
    
    numbytessrc=avpicture_get_size(fmtsrc, w, h);
    numbytesdst=avpicture_get_size(fmtdst, w, h);
    bufsrc  = new uint8_t[numbytessrc];
    bufdst  = new uint8_t[numbytesdst];
    avsrc = avcodec_alloc_frame();
    avdst = avcodec_alloc_frame();
    avpicture_fill((AVPicture*)avsrc,bufsrc,fmtsrc,w,h);
    avpicture_fill((AVPicture*)avdst,bufdst,fmtdst,w,h);
}

FfmpegConverter::~FfmpegConverter()
{
    sws_freeContext(img_convert_ctx);
    delete[] bufsrc;
    av_free(avsrc);
    delete[] bufdst;
    av_free(avdst);
}

void FfmpegConverter::Start()
{
    // No-Op
}

void FfmpegConverter::Stop()
{
    // No-Op
}

unsigned FfmpegConverter::Width() const
{
    return w;
}

unsigned FfmpegConverter::Height() const
{
    return h;
}

size_t FfmpegConverter::SizeBytes() const
{
    return numbytesdst;
}

VideoPixelFormat FfmpegConverter::PixFormat() const
{
    return VideoFormatFromString(FfmpegFmtToString(fmtdst));
}

bool FfmpegConverter::GrabNext( unsigned char* image, bool wait )
{
    if( videoin->GrabNext(avsrc->data[0],wait) )
    {
        sws_scale(
                    img_convert_ctx,
                    avsrc->data, avsrc->linesize, 0, h,
                    avdst->data, avdst->linesize
                    );
        memcpy(image,avdst->data[0],numbytesdst);
        return true;
    }
    return false;
}

bool FfmpegConverter::GrabNewest( unsigned char* image, bool wait )
{
    if( videoin->GrabNewest(avsrc->data[0],wait) )
    {
        sws_scale(
                    img_convert_ctx,
                    avsrc->data, avsrc->linesize, 0, h,
                    avdst->data, avdst->linesize
                    );
        memcpy(image,avdst->data[0],numbytesdst);
        return true;
    }
    return false;
}

// Based on this example
// http://cekirdek.pardus.org.tr/~ismail/ffmpeg-docs/output-example_8c-source.html
static AVStream* CreateStream(AVFormatContext *oc, enum CodecID codec_id, uint64_t frame_rate, int bit_rate, PixelFormat EncoderFormat, int width, int height)
{
    AVCodec* codec = avcodec_find_encoder(codec_id);
    if (!(codec)) throw
#if (LIBAVFORMAT_VERSION_MAJOR >= 54)
        VideoException("Could not find encoder", avcodec_get_name(codec_id));
#else
        VideoException("Could not find encoder");
#endif

    AVStream* stream = avformat_new_stream(oc, codec);
    if (!stream) throw VideoException("Could not allocate stream");
    
    stream->id = oc->nb_streams-1;
    
    switch (codec->type) {
//    case AVMEDIA_TYPE_AUDIO:
//        stream->id = 1;
//        stream->codec->sample_fmt  = AV_SAMPLE_FMT_S16;
//        stream->codec->bit_rate    = 64000;
//        stream->codec->sample_rate = 44100;
//        stream->codec->channels    = 2;
//        break;
    case AVMEDIA_TYPE_VIDEO:
        stream->codec->codec_id = codec_id;
        stream->codec->bit_rate = bit_rate;
        stream->codec->width    = width;
        stream->codec->height   = height;
        stream->codec->time_base.den = frame_rate;
        stream->codec->time_base.num = 1;
        stream->codec->gop_size      = 12;
        stream->codec->pix_fmt       = EncoderFormat;
        break;
    default:
        break;
    }
    
    /* Some formats want stream headers to be separate. */
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    
    /* open the codec */
    int ret = avcodec_open2(stream->codec, codec, NULL);
    if (ret < 0)  throw VideoException("Could not open video codec");
    
    return stream;
}

void FfmpegVideoOutputStream::WriteAvPacket(AVPacket* pkt)
{
    if (pkt->size) {
        pkt->stream_index = stream->index;
        int ret = av_interleaved_write_frame(recorder.oc, pkt);
        if (ret < 0) throw VideoException("Error writing video frame");
        last_pts = pkt->pts;
    }
}

void FfmpegVideoOutputStream::WriteFrame(AVFrame* frame)
{
    AVPacket pkt;
    pkt.data = NULL;
    pkt.size = 0;
    av_init_packet(&pkt);

    int ret;
    int got_packet = 1;

    // Setup AVPacket
    if (recorder.oc->oformat->flags & AVFMT_RAWPICTURE) {
        /* Raw video case - directly store the picture in the packet */
        pkt.flags        |= AV_PKT_FLAG_KEY;
        pkt.data          = frame->data[0];
        pkt.size          = sizeof(AVPicture);
        ret = 0;
    } else {
        /* encode the image */
#if (LIBAVFORMAT_VERSION_MAJOR >= 54)
        ret = avcodec_encode_video2(stream->codec, &pkt, frame, &got_packet);
#else
        ret = avcodec_encode_video(stream->codec,pkt.data, pkt.size, frame);
        got_packet = ret > 0;
#endif
        if (ret < 0) throw VideoException("Error encoding video frame");
    }
    
    if (!ret && got_packet) {
        WriteAvPacket(&pkt);
    }
    
    av_free_packet(&pkt);
}

void FfmpegVideoOutputStream::WriteImage(AVPicture& src_picture, int w, int h, PixelFormat fmt, int64_t pts)
{
    AVCodecContext *c = stream->codec;
    
    AVFrame* frame = avcodec_alloc_frame();
    frame->pts = pts;
    frame->width = w;
    frame->height = h;
        
    if (c->pix_fmt != fmt || c->width != w || c->height != h) {
        if(!sws_ctx) {
            sws_ctx = sws_getCachedContext( sws_ctx,
                w, h, fmt,
                c->width, c->height, c->pix_fmt,
                SWS_BICUBIC, NULL, NULL, NULL
            );
            if (!sws_ctx) throw VideoException("Could not initialize the conversion context");        
        }
        sws_scale(sws_ctx,
            src_picture.data, src_picture.linesize, 0, h,
            dst_picture.data, dst_picture.linesize
        );
        *((AVPicture *)frame) = dst_picture;
    } else {
        *((AVPicture *)frame) = src_picture;
    }    
    
    WriteFrame(frame);
    av_free(frame);
}

void FfmpegVideoOutputStream::WriteImage(uint8_t* img, int w, int h, const std::string& input_fmt, int64_t pts)
{
    recorder.StartStream();
    
    PixelFormat fmt = FfmpegFmtFromString(input_fmt);
    AVPicture picture;
    
    if(h < 0) {
        h = -h;
        avpicture_fill(&picture,img,fmt,w,h);        
        for(int i=0; i<4; ++i) {
            picture.data[i] += (h-1) * picture.linesize[i];
            picture.linesize[i] *= -1;
        }
    }else{
        avpicture_fill(&picture,img,fmt,w,h);        
    }

    WriteImage(picture, w, h, fmt, pts);
}

void FfmpegVideoOutputStream::WriteImage(uint8_t* img, int w, int h, const std::string& input_format, double time)
{
    const int64_t pts = (time >= 0) ? time / BaseFrameTime() : last_pts + 1;
    WriteImage(img,w,h,input_format,pts);
}

double FfmpegVideoOutputStream::BaseFrameTime()
{
    return (double)stream->codec->time_base.num / (double)stream->codec->time_base.den;
}

FfmpegVideoOutputStream::FfmpegVideoOutputStream(FfmpegVideoOutput& recorder, enum CodecID codec_id, uint64_t frame_rate, int bit_rate, PixelFormat EncoderFormat, int width, int height )
    : recorder(recorder), last_pts(-1), sws_ctx(NULL)
{
    int ret;

    stream = CreateStream(recorder.oc, codec_id, frame_rate, bit_rate, EncoderFormat, width, height);
        
    /* Allocate the encoded raw picture. */
    ret = avpicture_alloc(&dst_picture, stream->codec->pix_fmt, stream->codec->width, stream->codec->height);
    if (ret < 0) throw VideoException("Could not allocate picture");    
}

FfmpegVideoOutputStream::~FfmpegVideoOutputStream()
{
    if(sws_ctx) {
        sws_freeContext(sws_ctx);
    }
    
    av_free(dst_picture.data[0]);
    avcodec_close(stream->codec);
}

FfmpegVideoOutput::FfmpegVideoOutput(const std::string& filename, int base_frame_rate, int bit_rate)
    : filename(filename), started(false), oc(NULL),
      frame_count(0), base_frame_rate(base_frame_rate), bit_rate(bit_rate)
{
    Initialise(filename);
}

FfmpegVideoOutput::~FfmpegVideoOutput()
{
    Close();
}

void FfmpegVideoOutput::Initialise(std::string filename)
{
    av_register_all();

#if (LIBAVFORMAT_VERSION_MAJOR >= 54)
    int ret = avformat_alloc_output_context2(&oc, NULL, NULL, filename.c_str());
#else
    oc = avformat_alloc_context();
    oc->oformat = av_guess_format(NULL, filename.c_str(), NULL);
    int ret = oc->oformat ? 0 : -1;
#endif

    if (ret < 0 || !oc) {
        std::cout << "Could not deduce output format from file extension: using MPEG." << std::endl;
#if (LIBAVFORMAT_VERSION_MAJOR >= 54)
        ret = avformat_alloc_output_context2(&oc, NULL, "mpeg", filename.c_str());
#else
        oc->oformat = av_guess_format("mpeg", filename.c_str(), NULL);
#endif
        if (ret < 0 || !oc) throw VideoException("Couldn't create AVFormatContext");
    }
    
    /* open the output file, if needed */
    if (!(oc->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&oc->pb, filename.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0) throw VideoException("Could not open '%s'\n", filename);
    }    
}

void FfmpegVideoOutput::StartStream()
{
    if(!started) {
        av_dump_format(oc, 0, filename.c_str(), 1);
        
        /* Write the stream header, if any. */
        int ret = avformat_write_header(oc, NULL);
        if (ret < 0) throw VideoException("Error occurred when opening output file");
        
        started = true;
    }
}

void FfmpegVideoOutput::Close()
{
    av_write_trailer(oc);
    
    for(std::vector<FfmpegVideoOutputStream*>::iterator i = streams.begin(); i!=streams.end(); ++i)
    {
        delete *i;
    }
    
    if (!(oc->oformat->flags & AVFMT_NOFILE)) avio_close(oc->pb);

    avformat_free_context(oc);
}

void FfmpegVideoOutput::AddStream(int w, int h, const std::string& encoder_fmt)
{
    streams.push_back( new FfmpegVideoOutputStream(*this, oc->oformat->video_codec, base_frame_rate, bit_rate, FfmpegFmtFromString(encoder_fmt), w, h) );    
}

VideoOutputStreamInterface& FfmpegVideoOutput::operator[](size_t i)
{
    if(i < streams.size()) {
        return *streams[i];
    }
    throw VideoException("Attempt to access non-existent stream.");
}

}

