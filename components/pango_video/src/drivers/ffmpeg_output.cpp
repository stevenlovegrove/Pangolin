// It is impossible to keep up with ffmpeg deprecations, so ignore these warnings.
#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wdeprecated"
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <pangolin/video/drivers/ffmpeg_output.h>
#include <pangolin/factory/factory_registry.h>

namespace pangolin {

// Based on this example
// http://cekirdek.pardus.org.tr/~ismail/ffmpeg-docs/output-example_8c-source.html
static AVStream* CreateStream(AVFormatContext *oc, AVCodecID codec_id, uint64_t frame_rate, int bit_rate, AVPixelFormat EncoderFormat, int width, int height)
{
    AVCodec* codec = avcodec_find_encoder(codec_id);
    if (!(codec)) throw
        VideoException("Could not find encoder");

#if (LIBAVFORMAT_VERSION_MAJOR >= 54 || (LIBAVFORMAT_VERSION_MAJOR >= 53 && LIBAVFORMAT_VERSION_MINOR >= 21) )
    AVStream* stream = avformat_new_stream(oc, codec);
#else
    AVStream* stream = av_new_stream(oc, codec_id);
#endif

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
        stream->codec->time_base.num = 1;
        stream->codec->time_base.den = frame_rate;
        stream->codec->gop_size      = 12;
        stream->codec->pix_fmt       = EncoderFormat;
        break;
    default:
        break;
    }

    /* Some formats want stream headers to be separate. */
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    /* open the codec */
    int ret = avcodec_open2(stream->codec, codec, NULL);
    if (ret < 0)  throw VideoException("Could not open video codec");

    return stream;
}

class FfmpegVideoOutputStream
{
public:
    FfmpegVideoOutputStream(FfmpegVideoOutput& recorder, CodecID codec_id, uint64_t frame_rate, int bit_rate, const StreamInfo& input_info, bool flip );
    ~FfmpegVideoOutputStream();

    const StreamInfo& GetStreamInfo() const;

    void WriteImage(const uint8_t* img, int w, int h, double time);
    void Flush();

protected:
    void WriteAvPacket(AVPacket* pkt);
    void WriteFrame(AVFrame* frame);
    double BaseFrameTime();

    FfmpegVideoOutput& recorder;

    StreamInfo input_info;
    AVPixelFormat input_format;
    AVPixelFormat output_format;

    AVPicture src_picture;
    AVPicture dst_picture;
    int64_t last_pts;

    // These pointers are owned by class
    AVStream* stream;
    SwsContext *sws_ctx;
    AVFrame* frame;

    bool flip;
};

void FfmpegVideoOutputStream::WriteAvPacket(AVPacket* pkt)
{
    if (pkt->size) {
        pkt->stream_index = stream->index;
        int64_t pts = pkt->pts;
        /* convert unit from CODEC's timestamp to stream's one */
#define C2S(field)                                              \
        do {                                                    \
          if (pkt->field != (int64_t) AV_NOPTS_VALUE)           \
            pkt->field = av_rescale_q(pkt->field,               \
                                      stream->codec->time_base, \
                                      stream->time_base);       \
        } while (0)

        C2S(pts);
        C2S(dts);
        C2S(duration);
#undef C2S
        int ret = av_interleaved_write_frame(recorder.oc, pkt);
        if (ret < 0) throw VideoException("Error writing video frame");
        if(pkt->pts != (int64_t)AV_NOPTS_VALUE) last_pts = pts;
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

#if FF_API_LAVF_FMT_RAWPICTURE
    // Setup AVPacket
    if (recorder.oc->oformat->flags & AVFMT_RAWPICTURE) {
        /* Raw video case - directly store the picture in the packet */
        pkt.flags        |= AV_PKT_FLAG_KEY;
        pkt.data          = frame->data[0];
        pkt.size          = sizeof(AVPicture);
        pkt.pts           = frame->pts;
        ret = 0;
    } else {
#else
    {
#endif
        /* encode the image */
#if (LIBAVFORMAT_VERSION_MAJOR >= 54)
        ret = avcodec_encode_video2(stream->codec, &pkt, frame, &got_packet);
#else
        // TODO: Why is ffmpeg so fussy about this buffer size?
        //       Making this too big results in garbled output.
        //       Too small and it will fail entirely.
        pkt.size = 50* FF_MIN_BUFFER_SIZE; //std::max(FF_MIN_BUFFER_SIZE, frame->width * frame->height * 4 );
        // TODO: Make sure this is being freed by av_free_packet
        pkt.data = (uint8_t*) malloc(pkt.size);
        pkt.pts = frame->pts;
        ret = avcodec_encode_video(stream->codec, pkt.data, pkt.size, frame);
        got_packet = ret > 0;
#endif
        if (ret < 0) throw VideoException("Error encoding video frame");
    }

    if (ret >= 0 && got_packet) {
        WriteAvPacket(&pkt);
    }

    av_free_packet(&pkt);
}

void FfmpegVideoOutputStream::WriteImage(const uint8_t* img, int w, int h, double time=-1.0)
{
    const int64_t pts = (time >= 0) ? time / BaseFrameTime() : ++last_pts;

    recorder.StartStream();

    AVCodecContext *c = stream->codec;

    if(flip) {
        // Earlier versions of ffmpeg do not annotate img as const, although it is
        avpicture_fill(&src_picture,const_cast<uint8_t*>(img),input_format,w,h);
        for(int i=0; i<4; ++i) {
            src_picture.data[i] += (h-1) * src_picture.linesize[i];
            src_picture.linesize[i] *= -1;
        }
    }else{
        // Earlier versions of ffmpeg do not annotate img as const, although it is
        avpicture_fill(&src_picture,const_cast<uint8_t*>(img),input_format,w,h);
    }

    if (c->pix_fmt != input_format || c->width != w || c->height != h) {
        if(!sws_ctx) {
            sws_ctx = sws_getCachedContext( sws_ctx,
                w, h, input_format,
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

    frame->pts = pts;
    frame->width =  w;
    frame->height = h;
    frame->format = c->pix_fmt;
    WriteFrame(frame);
}

void FfmpegVideoOutputStream::Flush()
{
#if (LIBAVFORMAT_VERSION_MAJOR >= 54)
    if (stream->codec->codec->capabilities & AV_CODEC_CAP_DELAY) {
        /* some CODECs like H.264 needs flushing buffered frames by encoding NULL frames. */
        /* cf. https://www.ffmpeg.org/doxygen/trunk/group__lavc__encoding.html#ga2c08a4729f72f9bdac41b5533c4f2642 */

        AVPacket pkt;
        pkt.data = NULL;
        pkt.size = 0;
        av_init_packet(&pkt);

        int got_packet = 1;
        while (got_packet) {
            int ret = avcodec_encode_video2(stream->codec, &pkt, NULL, &got_packet);
            if (ret < 0) throw VideoException("Error encoding video frame");
            WriteAvPacket(&pkt);
        }

        av_free_packet(&pkt);
    }
#endif
}

const StreamInfo& FfmpegVideoOutputStream::GetStreamInfo() const
{
    return input_info;
}

double FfmpegVideoOutputStream::BaseFrameTime()
{
    return (double)stream->codec->time_base.num / (double)stream->codec->time_base.den;
}

FfmpegVideoOutputStream::FfmpegVideoOutputStream(
    FfmpegVideoOutput& recorder, CodecID codec_id, uint64_t frame_rate,
    int bit_rate, const StreamInfo& input_info, bool flip_image
)
    : recorder(recorder), input_info(input_info),
      input_format(FfmpegFmtFromString(input_info.PixFormat())),
      output_format( FfmpegFmtFromString("YUV420P") ),
      last_pts(-1), sws_ctx(NULL), frame(NULL), flip(flip_image)
{
    stream = CreateStream(recorder.oc, codec_id, frame_rate, bit_rate, output_format, input_info.Width(), input_info.Height() );

    // Allocate the encoded raw picture.
    int ret = avpicture_alloc(&dst_picture, stream->codec->pix_fmt, stream->codec->width, stream->codec->height);
    if (ret < 0) throw VideoException("Could not allocate picture");

    // Allocate frame
#if LIBAVUTIL_VERSION_MAJOR >= 54
    frame = av_frame_alloc();
#else
    // Deprecated
    frame = avcodec_alloc_frame();
#endif
}

FfmpegVideoOutputStream::~FfmpegVideoOutputStream()
{
    Flush();

    if(sws_ctx) {
        sws_freeContext(sws_ctx);
    }

    av_free(frame);
    av_free(dst_picture.data[0]);
    avcodec_close(stream->codec);
}

FfmpegVideoOutput::FfmpegVideoOutput(const std::string& filename, int base_frame_rate, int bit_rate, bool flip_image)
    : filename(filename), started(false), oc(NULL),
      frame_count(0), base_frame_rate(base_frame_rate), bit_rate(bit_rate), is_pipe(pangolin::IsPipe(filename)), flip(flip_image)
{
    Initialise(filename);
}

FfmpegVideoOutput::~FfmpegVideoOutput()
{
    Close();
}

bool FfmpegVideoOutput::IsPipe() const
{
    return is_pipe;
}

void FfmpegVideoOutput::Initialise(std::string filename)
{
    av_register_all();

#ifdef HAVE_FFMPEG_AVFORMAT_ALLOC_OUTPUT_CONTEXT2
    int ret = avformat_alloc_output_context2(&oc, NULL, NULL, filename.c_str());
#else
    oc = avformat_alloc_context();
    oc->oformat = av_guess_format(NULL, filename.c_str(), NULL);
    int ret = oc->oformat ? 0 : -1;
#endif

    if (ret < 0 || !oc) {
        pango_print_error("Could not deduce output format from file extension: using MPEG.\n");
#ifdef HAVE_FFMPEG_AVFORMAT_ALLOC_OUTPUT_CONTEXT2
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
#if (LIBAVFORMAT_VERSION_MAJOR >= 53)
        av_dump_format(oc, 0, filename.c_str(), 1);
#else
        // Deprecated
        dump_format(oc, 0, filename.c_str(), 1);
#endif

        /* Write the stream header, if any. */
        int ret = avformat_write_header(oc, NULL);
        if (ret < 0) throw VideoException("Error occurred when opening output file");

        started = true;
    }
}

void FfmpegVideoOutput::Close()
{
    for(std::vector<FfmpegVideoOutputStream*>::iterator i = streams.begin(); i!=streams.end(); ++i)
    {
        (*i)->Flush();
        delete *i;
    }

    av_write_trailer(oc);

    if (!(oc->oformat->flags & AVFMT_NOFILE)) avio_close(oc->pb);

    avformat_free_context(oc);
}

const std::vector<StreamInfo>& FfmpegVideoOutput::Streams() const
{
    return strs;
}

void FfmpegVideoOutput::SetStreams(const std::vector<StreamInfo>& str, const std::string& /*uri*/, const picojson::value& properties)
{
    strs.insert(strs.end(), str.begin(), str.end());

    for(std::vector<StreamInfo>::const_iterator i = str.begin(); i!= str.end(); ++i)
    {
        streams.push_back( new FfmpegVideoOutputStream(
            *this, oc->oformat->video_codec, base_frame_rate, bit_rate, *i, flip
        ) );
    }

    if(!properties.is<picojson::null>()) {
        pango_print_warn("Ignoring attached video properties.");
    }
}

int FfmpegVideoOutput::WriteStreams(const unsigned char* data, const picojson::value& /*frame_properties*/)
{
    for(std::vector<FfmpegVideoOutputStream*>::iterator i = streams.begin(); i!= streams.end(); ++i)
    {
        FfmpegVideoOutputStream& s = **i;
        Image<unsigned char> img = s.GetStreamInfo().StreamImage(data);
        s.WriteImage(img.ptr, img.w, img.h);
    }
    return frame_count++;
}

PANGOLIN_REGISTER_FACTORY(FfmpegVideoOutput)
{
    struct FfmpegVideoFactory final : public TypedFactoryInterface<VideoOutputInterface> {
        std::map<std::string,Precedence> Schemes() const override
        {
            return {{"ffmpeg",10}};
        }
        const char* Description() const override
        {
            return "Use FFMPEG lib to encode video";
        }
        ParamSet Params() const override
        {
            return {{
                {"fps","60","Playback frames-per-second to recommend in meta-data"},
                {"bps","20000*1024","desired bitrate (hint)"},
                {"flip","0","Flip the output vertically before recording"},
                {"unique_filename","","Automatically append a unique number instead of overwriting files"},
            }};
        }
        std::unique_ptr<VideoOutputInterface> Open(const Uri& uri) override {
            const int desired_frame_rate = uri.Get("fps", 60);
            const int desired_bit_rate = uri.Get("bps", 20000*1024);
            const bool flip = uri.Get("flip", false);
            std::string filename = uri.url;

            if(uri.Contains("unique_filename")) {
                filename = MakeUniqueFilename(filename);
            }

            return std::unique_ptr<VideoOutputInterface>(
                new FfmpegVideoOutput(filename, desired_frame_rate, desired_bit_rate, flip)
            );
        }
    };

    return FactoryRegistry::I()->RegisterFactory<VideoOutputInterface>(std::make_shared<FfmpegVideoFactory>());
}

}
