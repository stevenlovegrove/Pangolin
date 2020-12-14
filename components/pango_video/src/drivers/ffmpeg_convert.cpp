// It is impossible to keep up with ffmpeg deprecations, so ignore these warnings.
#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wdeprecated"
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <pangolin/video/video.h>
#include <pangolin/factory/factory_registry.h>
#include <pangolin/video/drivers/ffmpeg_convert.h>

namespace pangolin {

void FfmpegConverter::ConvertContext::convert(const unsigned char* src, unsigned char* dst)
{
    // avpicture_fill expects uint8_t* w/o const as the second parameter in earlier versions
    avpicture_fill((AVPicture*)avsrc, const_cast<unsigned char*>(src + src_buffer_offset), fmtsrc, w, h);
    avpicture_fill((AVPicture*)avdst, dst + dst_buffer_offset, fmtdst, w, h);
    sws_scale(  img_convert_ctx,
                avsrc->data, avsrc->linesize, 0, h,
                avdst->data, avdst->linesize         );
}

FfmpegConverter::FfmpegConverter(std::unique_ptr<VideoInterface> &videoin_, const std::string sfmtdst, FfmpegMethod method )
    :videoin(std::move(videoin_))
{
    if( !videoin )
        throw VideoException("Source video interface not specified");

    input_buffer = std::unique_ptr<unsigned char[]>(new unsigned char[videoin->SizeBytes()]);

    converters.resize(videoin->Streams().size());

    dst_buffer_size = 0;

    for(size_t i=0; i < videoin->Streams().size(); ++i) {
        const StreamInfo instrm = videoin->Streams()[i];

        converters[i].w=instrm.Width();
        converters[i].h=instrm.Height();

        converters[i].fmtdst = FfmpegFmtFromString(sfmtdst);
        converters[i].fmtsrc = FfmpegFmtFromString(instrm.PixFormat());
        converters[i].img_convert_ctx = sws_getContext(
            instrm.Width(), instrm.Height(), converters[i].fmtsrc,
            instrm.Width(), instrm.Height(), converters[i].fmtdst,
            method, NULL, NULL, NULL
        );
        if(!converters[i].img_convert_ctx)
            throw VideoException("Could not create SwScale context for pixel conversion");

        converters[i].dst_buffer_offset=dst_buffer_size;
        converters[i].src_buffer_offset=instrm.Offset() - (unsigned char*)0;
        //converters[i].src_buffer_offset=src_buffer_size;

        #if LIBAVUTIL_VERSION_MAJOR >= 54
            converters[i].avsrc = av_frame_alloc();
            converters[i].avdst = av_frame_alloc();
        #else
            // deprecated
            converters[i].avsrc = avcodec_alloc_frame();
            converters[i].avdst = avcodec_alloc_frame();
        #endif

        const PixelFormat pxfmtdst = PixelFormatFromString(sfmtdst);
        const StreamInfo sdst( pxfmtdst, instrm.Width(), instrm.Height(), (instrm.Width()*pxfmtdst.bpp)/8, (unsigned char*)0 + converters[i].dst_buffer_offset );
        streams.push_back(sdst);


        //src_buffer_size += instrm.SizeBytes();
        dst_buffer_size += avpicture_get_size(converters[i].fmtdst, instrm.Width(), instrm.Height());
    }

}

FfmpegConverter::~FfmpegConverter()
{
    for(ConvertContext&c:converters)
    {
        av_free(c.avsrc);
        av_free(c.avdst);
    }
}

void FfmpegConverter::Start()
{
    // No-Op
}

void FfmpegConverter::Stop()
{
    // No-Op
}

size_t FfmpegConverter::SizeBytes() const
{
    return dst_buffer_size;
}

const std::vector<StreamInfo>& FfmpegConverter::Streams() const
{
    return streams;
}

bool FfmpegConverter::GrabNext( unsigned char* image, bool wait )
{
    if( videoin->GrabNext(input_buffer.get(),wait) )
    {
        for(ConvertContext&c:converters) {
            c.convert(input_buffer.get(),image);
        }
        return true;
    }
    return false;
}

bool FfmpegConverter::GrabNewest( unsigned char* image, bool wait )
{
    if( videoin->GrabNewest(input_buffer.get(),wait) )
    {
        for(ConvertContext&c:converters) {
            c.convert(input_buffer.get(),image);
        }
        return true;
    }
    return false;
}

PANGOLIN_REGISTER_FACTORY(FfmpegVideoConvert)
{
    struct FfmpegVideoFactory : public TypedFactoryInterface<VideoInterface> {
        std::map<std::string,Precedence> Schemes() const override
        {
            return {{"ffmpeg_convert", 0}, {"convert", 20}};
        }
        const char* Description() const override
        {
            return "Use FFMPEG library to convert pixel format.";
        }
        ParamSet Params() const override
        {
            return {{
                {"fmt","RGB24","Pixel format: see pixel format help for all possible values"}
            }};
        }
        std::unique_ptr<VideoInterface> Open(const Uri& uri) override {
            std::string outfmt = uri.Get<std::string>("fmt","RGB24");
            ToUpper(outfmt);
            std::unique_ptr<VideoInterface> subvid = pangolin::OpenVideo(uri.url);
            return std::unique_ptr<VideoInterface>( new FfmpegConverter(subvid,outfmt,FFMPEG_POINT) );
        }
    };

    return FactoryRegistry::I()->RegisterFactory<VideoInterface>(std::make_shared<FfmpegVideoFactory>());
}

}
