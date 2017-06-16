#pragma once

#include <pangolin/platform.h>

#include <pangolin/video/video.h>
#include <pangolin/image/typed_image.h>

#ifdef HAVE_PNG
#include <png.h>
#endif // HAVE_PNG

#ifdef HAVE_JPEG
#include <jpeglib.h>
#ifndef HAVE_PNG
// This should not be included when HAVE_PNG, as png.h includes its own.
#include <setjmp.h>
#endif // HAVE_PNG
#endif // HAVE_JPEG

#ifdef _WIN_
// Undef windows Macro polution from jpeglib.h
#undef LoadImage
#endif

namespace pangolin {

class StreamEncoder
{
public:
    virtual ~StreamEncoder() {}
    virtual void Encode(std::vector<unsigned char>& encoded, const Image<unsigned char>& src_image) = 0;
};

struct jpeg_buffer_dest
{
    struct jpeg_destination_mgr jdst;
    JOCTET* buf;
    JOCTET* off;
    size_t sz;
    size_t used;
};

static void jpeg_buffer_dest_init(j_compress_ptr cinfo)
{
    struct jpeg_buffer_dest* dst = (struct jpeg_buffer_dest*)cinfo->dest;
    dst->used = 0;
    dst->sz = cinfo->image_width * cinfo->image_height * cinfo->input_components;
    dst->buf = (JOCTET*)malloc(dst->sz * sizeof *dst->buf);
    memset(dst->buf, 0, dst->sz);
    dst->off = dst->buf;
    dst->jdst.next_output_byte = dst->off;
    dst->jdst.free_in_buffer = dst->sz;
    return;
}

static boolean jpeg_buffer_dest_empty(j_compress_ptr cinfo)
{
    struct jpeg_buffer_dest* dst = (struct jpeg_buffer_dest*)cinfo->dest;
    dst->sz *= 2;
    dst->used = dst->off - dst->buf;
    dst->buf = (JOCTET*)realloc(dst->buf, dst->sz * sizeof *dst->buf);
    dst->off = dst->buf + dst->used;
    dst->jdst.next_output_byte = dst->off;
    dst->jdst.free_in_buffer = dst->sz - dst->used;
    return TRUE;
}

static void jpeg_buffer_dest_term(j_compress_ptr cinfo)
{
    struct jpeg_buffer_dest* dst = (struct jpeg_buffer_dest*)cinfo->dest;
    dst->used += dst->sz - dst->jdst.free_in_buffer;
    dst->off = dst->buf + dst->used;
    return;
}

static void jpeg_buffer_dest(j_compress_ptr cinfo, struct jpeg_buffer_dest* dst)
{
    dst->jdst.init_destination = jpeg_buffer_dest_init;
    dst->jdst.empty_output_buffer = jpeg_buffer_dest_empty;
    dst->jdst.term_destination = jpeg_buffer_dest_term;
    cinfo->dest = (struct jpeg_destination_mgr*)dst;
    return;
}

struct my_error_mgr2
{
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr2* my_error_ptr2;


static void init_source(j_decompress_ptr /*cinfo*/)
{
}

static uint8_t EOI_data[2] = {0xFF, 0xD9};

static boolean fill_input_buffer(j_decompress_ptr cinfo)
{
    cinfo->src->next_input_byte = EOI_data;
    cinfo->src->bytes_in_buffer = 2;
    return TRUE;
}

static void skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
    if(num_bytes > 0)
    {
        if(num_bytes > (long)cinfo->src->bytes_in_buffer)
            num_bytes = (long)cinfo->src->bytes_in_buffer;
        cinfo->src->next_input_byte += (size_t)num_bytes;
        cinfo->src->bytes_in_buffer -= (size_t)num_bytes;
    }
}

static void term_source(j_decompress_ptr /*cinfo*/)
{
}

//static void jpeg_buffer_src(j_decompress_ptr cinfo, unsigned char* buffer, long num)
//{
//    if(cinfo->src == NULL)
//    {
//        cinfo->src = (struct jpeg_source_mgr*)(*cinfo->mem->alloc_small)(
//                (j_common_ptr)cinfo, JPOOL_PERMANENT, sizeof(struct jpeg_source_mgr));
//    }

//    cinfo->src->init_source = init_source;
//    cinfo->src->fill_input_buffer = fill_input_buffer;
//    cinfo->src->skip_input_data = skip_input_data;
//    cinfo->src->resync_to_restart = jpeg_resync_to_restart; /* use default method */
//    cinfo->src->term_source = term_source;
//    cinfo->src->bytes_in_buffer = num;
//    cinfo->src->next_input_byte = (JOCTET*)buffer;
//}

//static void my_error_exit(j_common_ptr cinfo)
//{
//    my_error_ptr2 myerr = (my_error_ptr2)cinfo->err;
//    (*cinfo->err->output_message)(cinfo);
//    longjmp(myerr->setjmp_buffer, 1);
//}

//void DecodeGray8Jpeg(ManagedImage<unsigned char>& img, std::vector<unsigned char>& encoded)
//{
//    struct my_error_mgr2 jerr;
//    jerr.pub.error_exit = my_error_exit;

//    struct jpeg_decompress_struct cinfo;
//    cinfo.err = jpeg_std_error(&jerr.pub);

//    if(setjmp(jerr.setjmp_buffer))
//    {
//        // If we get here, the JPEG code has signaled an error.
//        jpeg_destroy_decompress(&cinfo);
//        PANGO_ENSURE(false);
//    }

//    jpeg_create_decompress(&cinfo);
//    jpeg_buffer_src(&cinfo, encoded.data(), encoded.size());
//    jpeg_read_header(&cinfo, TRUE);
//    jpeg_start_decompress(&cinfo);

//    PANGO_ENSURE(cinfo.jpeg_color_space == JCS_GRAYSCALE);

//    img.Reinitialise(cinfo.output_width, cinfo.output_height);

//    while(cinfo.output_scanline < cinfo.output_height)
//    {
//        unsigned char* row = img.RowPtr(cinfo.output_scanline);
//        jpeg_read_scanlines(&cinfo, &row, 1);
//    }

//    jpeg_finish_decompress(&cinfo);
//    jpeg_destroy_decompress(&cinfo);
//}


class JpegStreamEncoder : public StreamEncoder
{
public:
    JpegStreamEncoder(const PixelFormat& fmt, int quality)
        : fmt(fmt), quality(quality)
    {
        PANGO_ENSURE(quality >= 0 && quality <= 100);
        PANGO_ENSURE(fmt.bpp == 8 || fmt.bpp == 24);
    }

    void Encode(std::vector<unsigned char>& encoded, const Image<unsigned char>& src_image) override
    {
        struct jpeg_compress_struct comp;
        struct jpeg_error_mgr error;
        struct jpeg_buffer_dest dst;

        comp.err = jpeg_std_error(&error);
        jpeg_create_compress(&comp);
        jpeg_buffer_dest(&comp, &dst);
        comp.image_width = src_image.w;
        comp.image_height = src_image.h;
        comp.input_components = fmt.channels;

        if(fmt.format == "GRAY8")
        {
            comp.in_color_space = JCS_GRAYSCALE;
        }else if(fmt.format == "RGB24")
        {
            comp.in_color_space = JCS_RGB;
        }else{
            PANGO_ENSURE(false);
        }

        jpeg_set_defaults(&comp);
        jpeg_set_quality(&comp, quality, TRUE);
        jpeg_start_compress(&comp, TRUE);

        for(size_t y = 0; y < src_image.h; ++y)
        {
            unsigned char* row = const_cast<unsigned char*>(src_image.RowPtr(y));
            jpeg_write_scanlines(&comp, &row, 1);
        }

        jpeg_finish_compress(&comp);

        const size_t existing_size = encoded.size();
        encoded.resize(existing_size + dst.used);
        memcpy(encoded.data()+existing_size, dst.buf, dst.used);
        free(dst.buf);

        jpeg_destroy_compress(&comp);
    }

private:
    PixelFormat fmt;
    int quality;
};

class StreamEncoderFactory
{
public:
    static StreamEncoderFactory& I()
    {
        static StreamEncoderFactory instance;
        return instance;
    }

    std::unique_ptr<StreamEncoder> GetEncoder(const std::string& encoder_name, const PixelFormat& fmt)
    {
        if( encoder_name.substr(0,4) == "jpeg") {
            int quality = 100;

            try {
                quality = pangolin::Convert<int,std::string>::Do(encoder_name.substr(4));
            }catch(BadInputException)
            {
            }

            return std::unique_ptr<StreamEncoder>(new JpegStreamEncoder(fmt, quality));
        }else{
            PANGO_ENSURE(false);
            return nullptr;
        }
    }
};

}
