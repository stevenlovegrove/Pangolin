/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2014 Steven Lovegrove
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

#include <pangolin/video/drivers/shift.h>
#include <pangolin/factory/factory_registry.h>
#include <pangolin/video/iostream_operators.h>
#include <pangolin/video/video.h>

namespace pangolin
{

ShiftVideo::ShiftVideo(std::unique_ptr<VideoInterface>& src_,
                       const std::map<size_t, int>& shift_right_bits,
                       const std::map<size_t, uint32_t>& masks)
    : src(std::move(src_)), size_bytes(0), shift_right_bits(shift_right_bits), masks(masks)
{
    if(!src) {
        throw VideoException("ShiftVideo: VideoInterface in must not be null");
    }

    videoin.push_back(src.get());

    formats_supported.insert("GRAY16LE");
    formats_supported.insert("RGB48");
    formats_supported.insert("BGR48");
    formats_supported.insert("RGBA64");
    formats_supported.insert("BGRA64");

    for(size_t s=0; s< src->Streams().size(); ++s) {
        const size_t w = src->Streams()[s].Width();
        const size_t h = src->Streams()[s].Height();

        auto i = shift_right_bits.find(s);

        if(i != shift_right_bits.end() && i->second != 0)
        {
            if(formats_supported.count(src->Streams()[s].PixFormat().format) == 0)
            {
                throw VideoException("ShiftVideo: input format is not compatible for shifting.");
            }

            PixelFormat out_fmt;

            if(src->Streams()[s].PixFormat().format == "GRAY16LE")
            {
                out_fmt = PixelFormatFromString("GRAY8");
            }
            else if(src->Streams()[s].PixFormat().format == "RGB48")
            {
                out_fmt = PixelFormatFromString("RGB24");
            }
            else if(src->Streams()[s].PixFormat().format == "BGR48")
            {
                out_fmt = PixelFormatFromString("BGR24");
            }
            else if(src->Streams()[s].PixFormat().format == "RGBA64")
            {
                out_fmt = PixelFormatFromString("RGBA32");
            }
            else if(src->Streams()[s].PixFormat().format == "BGRA64")
            {
                out_fmt = PixelFormatFromString("BGRA32");
            }

            streams.push_back(pangolin::StreamInfo(out_fmt, w, h, w*out_fmt.bpp / 8, reinterpret_cast<uint8_t*>(size_bytes) ));
        }
        else
        {
            streams.push_back(pangolin::StreamInfo(src->Streams()[s].PixFormat(), w, h, w*src->Streams()[s].PixFormat().bpp / 8, reinterpret_cast<uint8_t*>(size_bytes) ));
        }

        size_bytes += streams.back().SizeBytes();
    }

    buffer.reset(new uint8_t[src->SizeBytes()]);
}

ShiftVideo::~ShiftVideo()
{
}

//! Implement VideoInput::Start()
void ShiftVideo::Start()
{
    videoin[0]->Start();
}

//! Implement VideoInput::Stop()
void ShiftVideo::Stop()
{
    videoin[0]->Stop();
}

//! Implement VideoInput::SizeBytes()
size_t ShiftVideo::SizeBytes() const
{
    return size_bytes;
}

//! Implement VideoInput::Streams()
const std::vector<StreamInfo>& ShiftVideo::Streams() const
{
    return streams;
}

void DoShift16to8(
    Image<uint8_t>& out,
    const Image<uint8_t>& in,
    const int shift_right_bits,
    const uint32_t mask,
    const uint16_t maxValBitDepth
) {
    for(size_t r = 0; r < out.h; ++r)
    {
        uint8_t* pout = (uint8_t*)(out.ptr + r*out.pitch);
        uint16_t* pin = (uint16_t*)(in.ptr + r*in.pitch);
        const uint16_t* pin_end = (uint16_t*)(in.ptr + (r+1)*in.pitch);
        while(pin != pin_end) {
            *(pout++) = (std::min(*(pin++), maxValBitDepth) >> shift_right_bits) & mask;
        }
    }
}

void ShiftVideo::Process(uint8_t* buffer_out, const uint8_t* buffer_in)
{
    for(size_t s=0; s<streams.size(); ++s) {
        const Image<uint8_t> img_in  = videoin[0]->Streams()[s].StreamImage(buffer_in);
        Image<uint8_t> img_out = Streams()[s].StreamImage(buffer_out);
        const size_t bytes_per_pixel = Streams()[s].PixFormat().bpp / 8;

        auto i = shift_right_bits.find(s);

        if(i != shift_right_bits.end() && i->second != 0)
        {
            auto m = masks.find(s);
            DoShift16to8(img_out, img_in, i->second, (m == masks.end() ? 0xffff : m->second), std::pow(2, videoin[0]->Streams()[s].PixFormat().channel_bit_depth) - 1);
        }
        else
        {
            //straight copy
            if( img_out.w != img_in.w || img_out.h != img_in.h ) {
                throw std::runtime_error("ShiftVideo: Incompatible image sizes");
            }

            for(size_t y=0; y < img_out.h; ++y) {
                std::memcpy(img_out.RowPtr((int)y), img_in.RowPtr((int)y), bytes_per_pixel * img_in.w);
            }
        }
    }
}

//! Implement VideoInput::GrabNext()
bool ShiftVideo::GrabNext( uint8_t* image, bool wait )
{
    if(videoin[0]->GrabNext(buffer.get(),wait)) {
        Process(image, buffer.get());
        return true;
    }else{
        return false;
    }
}

//! Implement VideoInput::GrabNewest()
bool ShiftVideo::GrabNewest( uint8_t* image, bool wait )
{
    if(videoin[0]->GrabNewest(buffer.get(),wait)) {
        Process(image, buffer.get());
        return true;
    }else{
        return false;
    }
}

std::vector<VideoInterface*>& ShiftVideo::InputStreams()
{
    return videoin;
}

PANGOLIN_REGISTER_FACTORY(ShiftVideo)
{
    struct ShiftVideoFactory final : public TypedFactoryInterface<VideoInterface> {
        std::map<std::string,Precedence> Schemes() const override
        {
            return {{"shift",10}};
        }
        const char* Description() const override
        {
            return "Video Filter: bitwise shift pixel values.";
        }
        ParamSet Params() const override
        {
            return {{
                {"shift\\d+","0","shiftN, N:[1,streams]. Right shift pixel values."},
                {"mask\\d+","65535","maskN, N:[1,streams]. Bitwise pixel mask (after shift)"},
            }};
        }
        std::unique_ptr<VideoInterface> Open(const Uri& uri) override {
            std::map<size_t, int> shift_right_bits;
            std::map<size_t, uint32_t> masks;

            ParamReader reader(Params(), uri);

            for(size_t i=0; i<100; ++i)
            {
                const std::string shift_key = pangolin::FormatString("shift%",i+1);
                const std::string mask_key = pangolin::FormatString("mask%",i+1);

                if(reader.Contains(shift_key))
                {
                    shift_right_bits[i] = reader.Get<int>(shift_key);
                }

                if(reader.Contains(mask_key))
                {
                    masks[i] = reader.Get<int>(mask_key);
                }
            }

            std::unique_ptr<VideoInterface> subvid = pangolin::OpenVideo(uri.url);
            return std::unique_ptr<VideoInterface>(
                new ShiftVideo(subvid, shift_right_bits, masks)
            );
        }
    };

    return FactoryRegistry::I()->RegisterFactory<VideoInterface>(std::make_shared<ShiftVideoFactory>());
}

}
