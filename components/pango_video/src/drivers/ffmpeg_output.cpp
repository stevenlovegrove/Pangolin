// It is impossible to keep up with ffmpeg deprecations, so ignore these warnings.
#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wdeprecated"
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <pangolin/video/drivers/ffmpeg_output.h>
#include <pangolin/factory/factory_registry.h>

namespace pangolin {

// Defined in ffmpeg.cpp
int pango_sws_scale_frame(struct SwsContext *c, AVFrame *dst, const AVFrame *src);

AVCodecContext* CreateVideoCodecContext(AVCodecID codec_id, uint64_t frame_rate, int bit_rate, AVPixelFormat EncoderFormat, int width, int height)
{
    const AVCodec* codec = avcodec_find_encoder(codec_id);
    if (!(codec))
        throw VideoException("Could not find encoder");

    if(codec->type != AVMEDIA_TYPE_VIDEO)
        throw VideoException("Encoder is not a video encoder");

    AVCodecContext* codec_context = avcodec_alloc_context3(codec);
    if(!codec_context)
        throw VideoException("Unable to create codec context");

    codec_context->codec_id = codec_id;
    codec_context->bit_rate = bit_rate;
    codec_context->width    = width;
    codec_context->height   = height;
    codec_context->time_base = av_make_q(1,frame_rate);
    codec_context->framerate = av_make_q(frame_rate,1);
    codec_context->gop_size      = 10;
    codec_context->max_b_frames  = 1;
    codec_context->pix_fmt       = EncoderFormat;

    /* open the codec */
    int ret = avcodec_open2(codec_context, nullptr, nullptr);
    if (ret < 0)  throw VideoException("Could not open video codec");

    return codec_context;
}

// Based on this example
// http://cekirdek.pardus.org.tr/~ismail/ffmpeg-docs/output-example_8c-source.html
static AVStream* CreateStream(AVFormatContext *oc, AVCodecContext* codec_context)
{
    /* Some formats want stream headers to be separate. */
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    AVStream* stream = avformat_new_stream(oc, codec_context->codec);
    if (!stream) throw VideoException("Could not allocate stream");

    stream->id = oc->nb_streams-1;
    stream->time_base = codec_context->time_base;
    stream->avg_frame_rate = codec_context->framerate;
    stream->r_frame_rate = stream->avg_frame_rate;
    stream->duration = codec_context->framerate.num * 60 / codec_context->framerate.den;
    avcodec_parameters_from_context(stream->codecpar, codec_context);

    return stream;
}

void FfmpegVideoOutputStream::WriteFrame(AVFrame* frame)
{
    AVPacket* pkt = av_packet_alloc();
    av_init_packet(pkt);
    pkt->data = NULL;
    pkt->size = 0;

    int ret;
    int got_packet = 1;

    /* encode the image */
    int response = avcodec_send_frame(codec_context, frame);
    while (response >= 0) {
       response = avcodec_receive_packet(codec_context, pkt);
       if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
         break;
       } else if (response < 0) {
         pango_print_error("Error while receiving packet from encoder.\n");
         return;
       }

       pkt->stream_index = stream->index;
//       pkt->pts = frame->pts;
//       pkt->dts = frame->pkt_dts;
//       pkt->time_base = codec_context->time_base;
       pkt->duration = 1; //av_div_q(codec_context->framerate, codec_context->time_base).num;
//       av_packet_rescale_ts(pkt, codec_context->time_base, pkt->time_base);

       if (pkt->size) {
           int64_t pts = pkt->pts;
           int ret = av_interleaved_write_frame(recorder.oc, pkt);
           if (ret < 0) throw VideoException("Error writing video frame");
           if(pkt->pts != (int64_t)AV_NOPTS_VALUE) last_pts = pts;
       }
     }
     av_packet_free(&pkt);
     return;
}

void FfmpegVideoOutputStream::WriteImage(const uint8_t* img, int w, int h)
{
    static int64_t pts = 0;

    av_frame_make_writable(src_frame);
    memcpy(src_frame->buf[0]->data, img, src_frame->buf[0]->size);

    recorder.StartStream();

    AVFrame* frame_to_write = nullptr;

    if (codec_context->pix_fmt != input_format || codec_context->width != w || codec_context->height != h) {
        if(!sws_ctx) {
            sws_ctx = sws_getCachedContext( sws_ctx,
                w, h, input_format,
                codec_context->width, codec_context->height, codec_context->pix_fmt,
                SWS_BICUBIC, NULL, NULL, NULL
            );
            if (!sws_ctx) throw VideoException("Could not initialize the conversion context");
        }
        av_frame_make_writable(frame);
        pango_sws_scale_frame(sws_ctx, frame, src_frame);
        frame_to_write = frame;
    } else {
        frame_to_write = src_frame;
    }

    if(frame_to_write) {
        frame_to_write->pts = pts;
        WriteFrame(frame_to_write);
        ++pts;
    }
}

void FfmpegVideoOutputStream::Flush()
{
    WriteFrame(nullptr);
}

const StreamInfo& FfmpegVideoOutputStream::GetStreamInfo() const
{
    return input_info;
}

double FfmpegVideoOutputStream::BaseFrameTime()
{
    return (double)codec_context->time_base.num / (double)codec_context->time_base.den;
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
    codec_context = CreateVideoCodecContext(codec_id, frame_rate, bit_rate, output_format, input_info.Width(), input_info.Height());
    stream = CreateStream(recorder.oc, codec_context);

    // Allocate frame
    frame = av_frame_alloc();
    frame->format = codec_context->pix_fmt;
    frame->width = codec_context->width;
    frame->height = codec_context->height;
    if(av_frame_get_buffer(frame,0)) {
        throw VideoException("Could not allocate picture");
    }

    src_frame = av_frame_alloc();
    src_frame->format = input_format;
    src_frame->width = input_info.Width();
    src_frame->height = input_info.Height();
    if(av_frame_get_buffer(src_frame,0)) {
        throw VideoException("Could not allocate picture");
    }

    if(flip) {
        // setup data pointer to end of memory, and negate line sizes.
        for(int i=0; i<4; ++i) {
            if(src_frame->data[i]) {
                src_frame->data[i] += (src_frame->height-1) * src_frame->linesize[i];
            }
            if(src_frame->linesize[i]) {
                src_frame->linesize[i] *= -1;
            }
        }
    }
}

FfmpegVideoOutputStream::~FfmpegVideoOutputStream()
{
    Flush();

    if(sws_ctx) {
        sws_freeContext(sws_ctx);
    }

    av_free(frame);
    avcodec_close(codec_context);
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
    int ret = avformat_alloc_output_context2(&oc, NULL, NULL, filename.c_str());

    if (ret < 0 || !oc) {
        pango_print_error("Could not deduce output format from file extension: using MPEG.\n");
        ret = avformat_alloc_output_context2(&oc, NULL, "mpeg", filename.c_str());
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
