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

#include "ffmpeg.h"

#ifdef HAVE_FFMPEG

namespace pangolin
{

PixelFormat FfmpegFmtFromString(const std::string fmt)
{
	std::string lfmt = boost::algorithm::to_lower_copy(fmt);
	return av_get_pix_fmt(lfmt.c_str());
}

#define TEST_PIX_FMT_RETURN(fmt) case PIX_FMT_##fmt: return #fmt;

std::string FfmpegFmtToString(const PixelFormat fmt)
{
	switch( fmt )
	{
	TEST_PIX_FMT_RETURN(YUV420P);
	TEST_PIX_FMT_RETURN(YUYV422);
//	TEST_PIX_FMT_RETURN(RGB24);
	case PIX_FMT_RGB24: return "RGB8";
//      TEST_PIX_FMT_RETURN(BGR24);
	case PIX_FMT_BGR24: return "BGR8";
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

FfmpegVideo::FfmpegVideo(const std::string filename, const std::string strfmtout)
    :pFormatCtx(0)
{
//    InitFile(filename, strfmtout);
    InitMjpeg("http://192.168.0.150/?action=stream", strfmtout);
}

void FfmpegVideo::InitFile(const std::string filename, const std::string strfmtout)
{
    if( filename.find('*') != filename.npos )
        throw VideoException("Wildcards not supported. Please use ffmpegs printf style formatting for image sequences. e.g. img-000000%04d.ppm");

    // Register all formats and codecs
    av_register_all();

    // Open video file
    if(av_open_input_file(&pFormatCtx, filename.c_str(), NULL, 0, NULL)!=0)
        throw VideoException("Couldn't open file");

    // Retrieve stream information
    if(av_find_stream_info(pFormatCtx)<0)
        throw VideoException("Couldn't find stream information");

    // Dump information about file onto standard error
    dump_format(pFormatCtx, 0, filename.c_str(), false);

    // Find the first video stream
    videoStream=-1;
    audioStream=-1;
    for(unsigned i=0; i<pFormatCtx->nb_streams; i++)
    {
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            videoStream=i;
        }else if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO)
        {
            audioStream=i;
        }
    }
    if(videoStream==-1)
        throw VideoException("Couldn't find a video stream");

    // Get a pointer to the codec context for the video stream
    pVidCodecCtx = pFormatCtx->streams[videoStream]->codec;

    // Find the decoder for the video stream
    pVidCodec=avcodec_find_decoder(pVidCodecCtx->codec_id);
    if(pVidCodec==0)
        throw VideoException("Codec not found");

    // Open video codec
    if(avcodec_open(pVidCodecCtx, pVidCodec)<0)
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

    PixelFormat fmtout = FfmpegFmtFromString(strfmtout);

    // Determine required buffer size and allocate buffer
    numBytesOut=avpicture_get_size(fmtout, pVidCodecCtx->width, pVidCodecCtx->height);

    buffer= new uint8_t[numBytesOut];

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    avpicture_fill((AVPicture *)pFrameOut, buffer, fmtout, pVidCodecCtx->width, pVidCodecCtx->height);

//    if( audioStream!=-1)
//    {
//        pAudCodecCtx = pFormatCtx->streams[audioStream]->codec;
//        pAudCodec = avcodec_find_decoder(pAudCodecCtx->codec_id);
//        if( pAudCodec!=0)
//        {
//            if(avcodec_open(pAudCodecCtx,pAudCodec) >= 0)
//            {
//                std::cout << "Found sound codec" << std::endl;
//            }
//        }
//    }
}

void FfmpegVideo::InitMjpeg(const std::string filename, const std::string strfmtout)
{
    // Register all formats and codecs
    av_register_all();

    std::cout << filename << std::endl;

    if( url_open(&pUrlCtx, filename.c_str(),0)!=0)
        throw VideoException("Couldn't url_open");

    std::cout << "1" << std::endl;

    if( url_fdopen(&pIoCtx,pUrlCtx) != 0 )
        throw VideoException("Couldn't url_fdopen");

    std::cout << "2" << std::endl;

    if(av_open_input_stream(&pFormatCtx, pIoCtx, filename.c_str(), NULL, NULL)!=0)
        throw VideoException("Couldn't open stream");

    std::cout << "3" << std::endl;

//    // Open video file
//    if(av_open_input_file(&pFormatCtx, filename.c_str(), NULL, 0, NULL)!=0)
//        throw VideoException("Couldn't open file");

    // Retrieve stream information
    if(av_find_stream_info(pFormatCtx)<0)
        throw VideoException("Couldn't find stream information");

    // Dump information about file onto standard error
    dump_format(pFormatCtx, 0, filename.c_str(), false);

    // Find the first video stream
    videoStream=-1;
    audioStream=-1;
    for(unsigned i=0; i<pFormatCtx->nb_streams; i++)
    {
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            videoStream=i;
        }else if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO)
        {
            audioStream=i;
        }
    }
    if(videoStream==-1)
        throw VideoException("Couldn't find a video stream");

    // Get a pointer to the codec context for the video stream
    pVidCodecCtx = pFormatCtx->streams[videoStream]->codec;

    // Find the decoder for the video stream
    pVidCodec=avcodec_find_decoder(pVidCodecCtx->codec_id);
    if(pVidCodec==0)
        throw VideoException("Codec not found");

    // Open video codec
    if(avcodec_open(pVidCodecCtx, pVidCodec)<0)
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

    PixelFormat fmtout = FfmpegFmtFromString(strfmtout);

    // Determine required buffer size and allocate buffer
    numBytesOut=avpicture_get_size(fmtout, pVidCodecCtx->width, pVidCodecCtx->height);

    buffer= new uint8_t[numBytesOut];

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    avpicture_fill((AVPicture *)pFrameOut, buffer, fmtout, pVidCodecCtx->width, pVidCodecCtx->height);
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
    av_close_input_file(pFormatCtx);
}


unsigned FfmpegVideo::Width() const
{
    return pVidCodecCtx->width;
}

unsigned FfmpegVideo::Height() const
{
    return pVidCodecCtx->height;
}

std::string FfmpegVideo::PixFormat() const
{
    return FfmpegFmtToString(pVidCodecCtx->pix_fmt);
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
        if(gotFrame)
        {
            static struct SwsContext *img_convert_ctx;

			if(img_convert_ctx == NULL) {
                                const int w = pVidCodecCtx->width;
                                const int h = pVidCodecCtx->height;

				img_convert_ctx = sws_getContext(w, h,
                                                                pVidCodecCtx->pix_fmt,
								w, h, PIX_FMT_RGB24, SWS_BICUBIC,
								NULL, NULL, NULL);
				if(img_convert_ctx == NULL) {
					fprintf(stderr, "Cannot initialize the conversion context!\n");
					exit(1);
				}
			}
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

std::string FfmpegConverter::PixFormat() const
{
    return FfmpegFmtToString(fmtdst);
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




}

#endif // HAVE_FFMPEG
