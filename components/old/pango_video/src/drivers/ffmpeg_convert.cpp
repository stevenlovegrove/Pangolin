// It is impossible to keep up with ffmpeg deprecations, so ignore these warnings.
#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wdeprecated"
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <pangolin/video/video.h>
#include <pangolin/factory/factory_registry.h>
#include <pangolin/video/drivers/ffmpeg_convert.h>

namespace pangolin {

// Defined in ffmpeg.cpp
int pango_sws_scale_frame(struct SwsContext *c, AVFrame *dst, const AVFrame *src);

void FfmpegConverter::ConvertContext::convert(const unsigned char* src, unsigned char* dst)
{
    // Copy into ffmpeg src buffer from user buffer
    memcpy(avsrc->buf[0]->data, src + src_buffer_offset, avsrc->buf[0]->size);
    pango_sws_scale_frame(img_convert_ctx, avdst, avsrc);
    av_image_copy_to_buffer(dst + dst_buffer_offset, avdst->buf[0]->size, avdst->data, avdst->linesize, fmtdst, avdst->width, avdst->height, 1);
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

        converters[i].avsrc = av_frame_alloc();
        converters[i].avsrc->width = instrm.Width();
        converters[i].avsrc->height = instrm.Height();
        converters[i].avsrc->format = FfmpegFmtFromString(instrm.PixFormat());
        av_frame_get_buffer(converters[i].avsrc, 0);

        converters[i].avdst = av_frame_alloc();
        converters[i].avdst->width = instrm.Width();
        converters[i].avdst->height = instrm.Height();
        converters[i].avdst->format = FfmpegFmtFromString(sfmtdst);
        av_frame_get_buffer(converters[i].avdst, 0);

        const PixelFormat pxfmtdst = PixelFormatFromString(sfmtdst);
        const StreamInfo sdst( pxfmtdst, instrm.Width(), instrm.Height(), (instrm.Width()*pxfmtdst.bpp)/8, (unsigned char*)0 + converters[i].dst_buffer_offset );
        streams.push_back(sdst);

        dst_buffer_size += av_image_get_buffer_size(converters[i].fmtdst, instrm.Width(), instrm.Height(), 0);
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
            c.convert(input_buffer.get(), image);
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
