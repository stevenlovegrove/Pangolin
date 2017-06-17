#pragma once

#include <memory>

#include <pangolin/image/image_io.h>

namespace pangolin {

using ImageEncoderFunc = std::function<void(std::ostream&, const Image<unsigned char>&)>;
using ImageDecoderFunc = std::function<TypedImage(std::istream&)>;

class StreamEncoderFactory
{
public:
    static StreamEncoderFactory& I()
    {
        static StreamEncoderFactory instance;
        return instance;
    }

    ImageEncoderFunc GetEncoder(const std::string& encoder_name, const PixelFormat& fmt)
    {
        if( encoder_name.substr(0,4) == "jpeg") {
            return [fmt](std::ostream& os, const Image<unsigned char>& img){
                SaveImage(img,fmt,os,ImageFileTypeJpg);
            };
        }else{
            PANGO_ENSURE(false);
            return nullptr;
        }
    }

    ImageDecoderFunc GetDecoder(const std::string& encoder_name, const PixelFormat& fmt)
    {
        if( encoder_name.substr(0,4) == "jpeg") {
            return [fmt](std::istream& is){
                return LoadImage(is,ImageFileTypeJpg);
            };
        }else{
            PANGO_ENSURE(false);
            return nullptr;
        }
    }
};

}
