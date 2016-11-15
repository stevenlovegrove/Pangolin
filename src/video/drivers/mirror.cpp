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

#include <pangolin/video/drivers/mirror.h>
#include <pangolin/video/video_factory.h>
#include <pangolin/video/iostream_operators.h>

namespace pangolin
{

MirrorVideo::MirrorVideo(std::unique_ptr<VideoInterface>& src, const std::vector<MirrorOptions>& flips)
    : videoin(std::move(src)), flips(flips), size_bytes(0),buffer(0)
{
    if(!videoin) {
        throw VideoException("MirrorVideo: VideoInterface in must not be null");
    }

    inputs.push_back(videoin.get());

    streams = videoin->Streams();
    size_bytes = videoin->SizeBytes();
    buffer = new unsigned char[size_bytes];
}

MirrorVideo::~MirrorVideo()
{
    delete[] buffer;
}

//! Implement VideoInput::Start()
void MirrorVideo::Start()
{
    videoin->Start();
}

//! Implement VideoInput::Stop()
void MirrorVideo::Stop()
{
    videoin->Stop();
}

//! Implement VideoInput::SizeBytes()
size_t MirrorVideo::SizeBytes() const
{
    return size_bytes;
}

//! Implement VideoInput::Streams()
const std::vector<StreamInfo>& MirrorVideo::Streams() const
{
    return streams;
}

void PitchedImageCopy(
    Image<unsigned char>& img_out,
    const Image<unsigned char>& img_in,
    size_t bytes_per_pixel
) {
    if( img_out.w != img_in.w || img_out.h != img_in.h ) {
        throw std::runtime_error("PitchedImageCopy: Incompatible image sizes");
    }

    for(size_t y=0; y < img_out.h; ++y) {
        std::memcpy(img_out.RowPtr((int)y), img_in.RowPtr((int)y), bytes_per_pixel * img_in.w);
    }
}

void FlipY(
    Image<unsigned char>& img_out,
    const Image<unsigned char>& img_in,
    size_t bytes_per_pixel
) {
    if( img_out.w != img_in.w || img_out.h != img_in.h ) {
        throw std::runtime_error("FlipY: Incompatible image sizes");
    }

    for(size_t y_out=0; y_out < img_out.h; ++y_out) {
        const size_t y_in = (img_in.h-1) - y_out;
        std::memcpy(img_out.RowPtr((int)y_out), img_in.RowPtr((int)y_in), bytes_per_pixel * img_in.w);
    }
}

void FlipX(
    Image<unsigned char>& img_out,
    const Image<unsigned char>& img_in,
    size_t bytes_per_pixel
) {
    for(size_t y=0; y < img_out.h; ++y) {
        for(size_t x=0; x < img_out.w; ++x) {
            memcpy(
                img_out.RowPtr((int)y) + (img_out.w-1-x)*bytes_per_pixel,
                img_in.RowPtr((int)y)  + x*bytes_per_pixel,
                bytes_per_pixel
            );
        }
    }
}

void FlipXY(
    Image<unsigned char>& img_out,
    const Image<unsigned char>& img_in,
    size_t bytes_per_pixel
) {
    for(size_t y_out=0; y_out < img_out.h; ++y_out) {
        for(size_t x=0; x < img_out.w; ++x) {
            const size_t y_in = (img_in.h-1) - y_out;
            memcpy(
                img_out.RowPtr((int)y_out) + (img_out.w-1-x)*bytes_per_pixel,
                img_in.RowPtr((int)y_in)   + x*bytes_per_pixel,
                bytes_per_pixel
            );
        }
    }
}

void MirrorVideo::Process(unsigned char* buffer_out, const unsigned char* buffer_in)
{
    for(size_t s=0; s<streams.size(); ++s) {
        Image<unsigned char> img_out = Streams()[s].StreamImage(buffer_out);
        const Image<unsigned char> img_in  = videoin->Streams()[s].StreamImage(buffer_in);
        const size_t bytes_per_pixel = Streams()[s].PixFormat().bpp / 8;

        switch (flips[s]) {
        case MirrorOptionsFlipX:
            FlipX(img_out, img_in, bytes_per_pixel);
            break;
        case MirrorOptionsFlipY:
            FlipY(img_out, img_in, bytes_per_pixel);
            break;
        case MirrorOptionsFlipXY:
            FlipXY(img_out, img_in, bytes_per_pixel);
            break;
        default:
            pango_print_warn("MirrorVideo::Process(): Invalid enum %i.\n", flips[s]);
        case MirrorOptionsNone:
            PitchedImageCopy(img_out, img_in, bytes_per_pixel);
            break;
        }
    }
}

//! Implement VideoInput::GrabNext()
bool MirrorVideo::GrabNext( unsigned char* image, bool wait )
{    
    if(videoin->GrabNext(buffer,wait)) {
        Process(image, buffer);
        return true;
    }else{
        return false;
    }
}

//! Implement VideoInput::GrabNewest()
bool MirrorVideo::GrabNewest( unsigned char* image, bool wait )
{
    if(videoin->GrabNewest(buffer,wait)) {
        Process(image, buffer);
        return true;
    }else{
        return false;
    }
}

std::vector<VideoInterface*>& MirrorVideo::InputStreams()
{
    return inputs;
}

unsigned int MirrorVideo::AvailableFrames() const
{
    BufferAwareVideoInterface* vpi = dynamic_cast<BufferAwareVideoInterface*>(videoin.get());
    if(!vpi)
    {
        pango_print_warn("Mirror: child interface is not buffer aware.");
        return 0;
    }
    else
    {
        return vpi->AvailableFrames();
    }
}

bool MirrorVideo::DropNFrames(uint32_t n)
{
    BufferAwareVideoInterface* vpi = dynamic_cast<BufferAwareVideoInterface*>(videoin.get());
    if(!vpi)
    {
        pango_print_warn("Mirror: child interface is not buffer aware.");
        return false;
    }
    else
    {
        return vpi->DropNFrames(n);
    }
}

std::istream& operator>> (std::istream &is, MirrorOptions &mirror)
{
    std::string str_mirror;
    is >> str_mirror;
    std::transform(str_mirror.begin(), str_mirror.end(), str_mirror.begin(), toupper);

    if(!str_mirror.compare("NONE")) {
        mirror = MirrorOptionsNone;
    }else if(!str_mirror.compare("FLIPX")) {
        mirror = MirrorOptionsFlipX;
    }else if(!str_mirror.compare("FLIPY")) {
        mirror = MirrorOptionsFlipY;
    }else if(!str_mirror.compare("FLIPXY")) {
        mirror = MirrorOptionsFlipXY;
    }else{
        pango_print_warn("Unknown mirror option %s.", str_mirror.c_str());
        mirror = MirrorOptionsNone;
    }

    return is;
}

PANGOLIN_REGISTER_FACTORY(MirrorVideo)
{
    struct MirrorVideoFactory : public VideoFactoryInterface {
        std::unique_ptr<VideoInterface> OpenVideo(const Uri& uri) override {
            std::unique_ptr<VideoInterface> subvid = pangolin::OpenVideo(uri.url);

            MirrorOptions default_opt = MirrorOptionsFlipX;
            if(uri.scheme == "flip") default_opt = MirrorOptionsFlipY;
            if(uri.scheme == "rotate") default_opt = MirrorOptionsFlipXY;

            std::vector<MirrorOptions> flips;

            for(size_t i=0; i < subvid->Streams().size(); ++i){
                std::stringstream ss;
                ss << "stream" << i;
                const std::string key = ss.str();
                flips.push_back(uri.Get<MirrorOptions>(key, default_opt) );
            }

            return std::unique_ptr<VideoInterface> (new MirrorVideo(subvid, flips));
        }
    };

    auto factory = std::make_shared<MirrorVideoFactory>();
    VideoFactoryRegistry::I().RegisterFactory(factory, 10, "mirror");
    VideoFactoryRegistry::I().RegisterFactory(factory, 10, "flip");
    VideoFactoryRegistry::I().RegisterFactory(factory, 10, "rotate");
}

}
