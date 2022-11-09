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

#include <pangolin/video/drivers/gamma.h>
#include <pangolin/factory/factory_registry.h>
#include <pangolin/video/iostream_operators.h>
#include <pangolin/utils/avx_math.h>

namespace pangolin
{


GammaVideo::GammaVideo(std::unique_ptr<VideoInterface>& src_, const std::map<size_t, float> &stream_gammas)
    : src(std::move(src_)), size_bytes(0), stream_gammas(stream_gammas)
{
    if(!src.get()) {
        throw VideoException("GammaVideo: VideoInterface in must not be null");
    }

    videoin.push_back(src.get());

    formats_supported.insert("GRAY8");
    formats_supported.insert("GRAY16LE");
    formats_supported.insert("RGB24");
    formats_supported.insert("BGR24");
    formats_supported.insert("RGB48");
    formats_supported.insert("BGR48");
    formats_supported.insert("RGBA32");
    formats_supported.insert("BGRA32");
    formats_supported.insert("RGBA64");
    formats_supported.insert("BGRA64");

    for(size_t s = 0; s < src->Streams().size(); s++)
    {
        auto i = stream_gammas.find(s);

        if(i != stream_gammas.end() && i->second != 0.0f && i->second != 1.0f &&
           formats_supported.count(src->Streams()[s].PixFormat().format) == 0)
        {
            throw VideoException("GammaVideo: Stream format not supported");
        }

        streams.push_back(src->Streams()[s]);
        size_bytes += streams.back().SizeBytes();
    }

    buffer.reset(new uint8_t[src->SizeBytes()]);
}

GammaVideo::~GammaVideo()
{
}

//! Implement VideoInput::Start()
void GammaVideo::Start()
{
    videoin[0]->Start();
}

//! Implement VideoInput::Stop()
void GammaVideo::Stop()
{
    videoin[0]->Stop();
}

//! Implement VideoInput::SizeBytes()
size_t GammaVideo::SizeBytes() const
{
    return size_bytes;
}

//! Implement VideoInput::Streams()
const std::vector<StreamInfo>& GammaVideo::Streams() const
{
    return streams;
}

template <typename T>
void ApplyGamma(Image<uint8_t>& out,
                       const Image<uint8_t>& in,
                       const float gamma,
                       const float channel_max_value);

#ifdef __AVX2__
template <>
void ApplyGamma<uint8_t>(Image<uint8_t>& out,
                         const Image<uint8_t>& in,
                         const float gamma,
                         const float channel_max_value)
{
    //Special shuffling constants for bytes across lanes
    const __m256i K0 = _mm256_setr_epi8(
        0x70u, 0x70u, 0x70u, 0x70u, 0x70u, 0x70u, 0x70u, 0x70u, 0x70u, 0x70u, 0x70u, 0x70u, 0x70u, 0x70u, 0x70u, 0x70u,
        0xF0u, 0xF0u, 0xF0u, 0xF0u, 0xF0u, 0xF0u, 0xF0u, 0xF0u, 0xF0u, 0xF0u, 0xF0u, 0xF0u, 0xF0u, 0xF0u, 0xF0u, 0xF0u);

    const __m256i K1 = _mm256_setr_epi8(
        0xF0u, 0xF0u, 0xF0u, 0xF0u, 0xF0u, 0xF0u, 0xF0u, 0xF0u, 0xF0u, 0xF0u, 0xF0u, 0xF0u, 0xF0u, 0xF0u, 0xF0u, 0xF0u,
        0x70u, 0x70u, 0x70u, 0x70u, 0x70u, 0x70u, 0x70u, 0x70u, 0x70u, 0x70u, 0x70u, 0x70u, 0x70u, 0x70u, 0x70u, 0x70u);

    const __m256i shuffle = _mm256_setr_epi8(0, 4, 8, 12, 16, 20, 24, 28, 2, 9, 10, 11, 3, 13, 14, 15, 4,
        17, 18, 19, 5, 21, 22, 23, 6, 25, 26, 27, 7, 29, 30, 31);

    for(size_t r = 0; r < out.h; ++r)
    {
        uint8_t* pout = out.ptr + r * out.pitch;
        uint8_t* pin = in.ptr + r * in.pitch;
        const uint8_t* pin_end = in.ptr + (r + 1) * in.pitch;
        const uint32_t numElems = in.pitch / sizeof(uint8_t);

        constexpr int vecElems = 8;

        const int numVecs = (numElems / vecElems) * vecElems;

        //Processing eight at a time (max number of floats per vector)
        for(int vec = 0; vec < numVecs; vec += vecElems, pin += vecElems, pout += vecElems)
        {
            //Convert bytes to floats
            const __m256 floatVals = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(_mm_cvtsi64_si128(*(uint64_t*)pin)));

            //Apply gamma and prepare for truncation (rounding) to integer
            const __m256 gammaValues = _mm256_add_ps(_mm256_mul_ps(pow256_ps(_mm256_div_ps(floatVals, _mm256_set1_ps(channel_max_value)), _mm256_set1_ps(gamma)), _mm256_set1_ps(channel_max_value)), _mm256_set1_ps(0.5f));

            //Clamp and convert to integer
            __m256i gammaValuesI = _mm256_min_epi32(_mm256_max_epi32(_mm256_cvtps_epi32(gammaValues), _mm256_set1_epi32(0)), _mm256_set1_epi32(255));

            //Unshuffle bytes to end of vector
            gammaValuesI = _mm256_or_si256(_mm256_shuffle_epi8(gammaValuesI, _mm256_add_epi8(shuffle, K0)),
                                           _mm256_shuffle_epi8(_mm256_permute4x64_epi64(gammaValuesI, 0x4E), _mm256_add_epi8(shuffle, K1)));

            //Copy result out
            *(uint64_t*)pout = _mm_cvtsi128_si64(_mm256_castsi256_si128(gammaValuesI));
        }

        //Remainder loose ends
        while(pin != pin_end) {
            *(pout++) = uint8_t(std::pow(float(*(pin++)) / channel_max_value, gamma) * channel_max_value + 0.5f);
        }
    }
}

template <>
void ApplyGamma<uint16_t>(Image<uint8_t>& out,
                          const Image<uint8_t>& in,
                          const float gamma,
                          const float channel_max_value)
{
    for(size_t r = 0; r < out.h; ++r)
    {
        uint16_t* pout = (uint16_t*)(out.ptr + r * out.pitch);
        uint16_t* pin = (uint16_t*)(in.ptr + r * in.pitch);
        const uint16_t* pin_end = (uint16_t*)(in.ptr + (r + 1) * in.pitch);
        const uint32_t numElems = in.pitch / sizeof(uint16_t);

        constexpr int vecElems = 8;

        const int numVecs = (numElems / vecElems) * vecElems;

        //Processing eight at a time (max number of floats per vector)
        for(int vec = 0; vec < numVecs; vec += vecElems, pin += vecElems, pout += vecElems)
        {
            //Convert shorts to floats
            const __m256 floatVals = _mm256_cvtepi32_ps(_mm256_cvtepu16_epi32(_mm_loadu_si128((const __m128i*)pin)));

            //Apply gamma and prepare for truncation (rounding) to integer
            const __m256 gammaValues = _mm256_add_ps(_mm256_mul_ps(pow256_ps(_mm256_div_ps(floatVals, _mm256_set1_ps(channel_max_value)), _mm256_set1_ps(gamma)), _mm256_set1_ps(channel_max_value)), _mm256_set1_ps(0.5f));

            //Clamp and convert to integer
            __m256i gammaValuesI = _mm256_min_epi32(_mm256_max_epi32(_mm256_cvtps_epi32(gammaValues), _mm256_set1_epi32(0)), _mm256_set1_epi32(65535));

            //Unshuffle shorts to end of vector
            gammaValuesI = _mm256_packs_epi32(gammaValuesI, _mm256_setzero_si256());
            gammaValuesI = _mm256_permute4x64_epi64(gammaValuesI, 0xD8);

            // Copy result out
            *(__m128i*)pout = _mm256_castsi256_si128(gammaValuesI);
        }

        //Remainder loose ends
        while(pin != pin_end) {
            *(pout++) = uint16_t(std::pow(float(*(pin++)) / channel_max_value, gamma) * channel_max_value + 0.5f);
        }
    }
}
#else
template <typename T>
void ApplyGamma(Image<uint8_t>& out,
                const Image<uint8_t>& in,
                const float gamma,
                const float channel_max_value)
{
    for(size_t r = 0; r < out.h; ++r)
    {
        T* pout = (T*)(out.ptr + r*out.pitch);
        T* pin = (T*)(in.ptr + r*in.pitch);
        const T* pin_end = (T*)(in.ptr + (r+1)*in.pitch);
        while(pin != pin_end) {
            *(pout++) = T(std::pow(float(*(pin++)) / channel_max_value, gamma) * channel_max_value + 0.5f);
        }
    }
}
#endif

void GammaVideo::Process(uint8_t* buffer_out, const uint8_t* buffer_in)
{
    for(size_t s=0; s<streams.size(); ++s) {
        Image<uint8_t> img_out = Streams()[s].StreamImage(buffer_out);
        const Image<uint8_t> img_in  = videoin[0]->Streams()[s].StreamImage(buffer_in);
        const size_t bytes_per_pixel = Streams()[s].PixFormat().bpp / 8;

        auto i = stream_gammas.find(s);

        if(i != stream_gammas.end() && i->second != 0.0f && i->second != 1.0f)
        {
            const float gamma = i->second;

            if(Streams()[s].PixFormat().format == "GRAY8" ||
               Streams()[s].PixFormat().format == "RGB24" ||
               Streams()[s].PixFormat().format == "BGR24" ||
               Streams()[s].PixFormat().format == "RGBA32" ||
               Streams()[s].PixFormat().format == "BGRA32")
            {
                ApplyGamma<uint8_t>(img_out, img_in, gamma, std::pow(2, Streams()[s].PixFormat().channel_bit_depth) - 1);
            }
            else if(Streams()[s].PixFormat().format == "GRAY16LE" ||
                    Streams()[s].PixFormat().format == "RGB48" ||
                    Streams()[s].PixFormat().format == "BGR48" ||
                    Streams()[s].PixFormat().format == "RGBA64" ||
                    Streams()[s].PixFormat().format == "BGRA64")
            {
                ApplyGamma<uint16_t>(img_out, img_in, gamma, std::pow(2, Streams()[s].PixFormat().channel_bit_depth) - 1);
            }
            else
            {
                throw VideoException("GammaVideo: Stream format not supported");
            }
        }
        else
        {
            //straight copy
            if( img_out.w != img_in.w || img_out.h != img_in.h ) {
                throw std::runtime_error("GammaVideo: Incompatible image sizes");
            }

            for(size_t y=0; y < img_out.h; ++y) {
                std::memcpy(img_out.RowPtr((int)y), img_in.RowPtr((int)y), bytes_per_pixel * img_in.w);
            }
        }
    }
}

//! Implement VideoInput::GrabNext()
bool GammaVideo::GrabNext( uint8_t* image, bool wait )
{
    if(videoin[0]->GrabNext(buffer.get(),wait)) {
        Process(image, buffer.get());
        return true;
    }else{
        return false;
    }
}

//! Implement VideoInput::GrabNewest()
bool GammaVideo::GrabNewest( uint8_t* image, bool wait )
{
    if(videoin[0]->GrabNewest(buffer.get(),wait)) {
        Process(image, buffer.get());
        return true;
    }else{
        return false;
    }
}

std::vector<VideoInterface*>& GammaVideo::InputStreams()
{
    return videoin;
}

uint32_t GammaVideo::AvailableFrames() const
{
    BufferAwareVideoInterface* vpi = dynamic_cast<BufferAwareVideoInterface*>(videoin[0]);
    if(!vpi)
    {
        pango_print_warn("Gamma: child interface is not buffer aware.");
        return 0;
    }
    else
    {
        return vpi->AvailableFrames();
    }
}

bool GammaVideo::DropNFrames(uint32_t n)
{
    BufferAwareVideoInterface* vpi = dynamic_cast<BufferAwareVideoInterface*>(videoin[0]);
    if(!vpi)
    {
        pango_print_warn("Gamma: child interface is not buffer aware.");
        return false;
    }
    else
    {
        return vpi->DropNFrames(n);
    }
}

PANGOLIN_REGISTER_FACTORY(GammaVideo)
{
    struct GammaVideoFactory final : public TypedFactoryInterface<VideoInterface> {
        GammaVideoFactory()
        {
            param_set_ = {{
                {"gamma\\d+","1.0","gammaK, where 1 <= K <= N where N is the number of streams"}
            }};
        }
        std::unique_ptr<VideoInterface> Open(const Uri& uri) override {

            ParamReader reader(param_set_, uri);
            // Gamma for each stream
            std::map<size_t, float> stream_gammas;
            for(size_t i=0; i<100; ++i)
            {
                const std::string gamma_key = pangolin::FormatString("gamma%",i+1);

                if(uri.Contains(gamma_key))
                {
                    stream_gammas[i] = reader.Get<float>(gamma_key, 1.0f);
                }
            }

            std::unique_ptr<VideoInterface> subvid = pangolin::OpenVideo(uri.url);

            return std::unique_ptr<VideoInterface> (new GammaVideo(subvid, stream_gammas));
        }
        FactoryUseInfo Help( const std::string& scheme ) const override {
            return FactoryUseInfo(scheme, "Gamma corrects a set of video streams", param_set_);
        }

        bool ValidateUri( const std::string& scheme, const Uri& uri, std::unordered_set<std::string>& unrecognized_params) const override {
            return ValidateUriAgainstParamSet(scheme, param_set_, uri, unrecognized_params );
        }

        bool IsValidated( const std::string& ) const override {return true;}

        ParamSet param_set_;
    };

    auto factory = std::make_shared<GammaVideoFactory>();
    FactoryRegistry::I()->RegisterFactory<VideoInterface>(factory, 10, "gamma");
}

}
