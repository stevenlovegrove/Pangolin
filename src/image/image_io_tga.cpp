#include <pangolin/platform.h>

#include <pangolin/image/image_io.h>

namespace pangolin {

PixelFormat TgaFormat(int depth, int color_type, int color_map)
{
    if(color_map == 0) {
        if(color_type == 2) {
            // Colour
            if(depth == 24) {
                return PixelFormatFromString("RGB24");
            }else if(depth == 32) {
                return PixelFormatFromString("RGBA32");
            }
        }else if(color_type == 3){
            // Greyscale
            if(depth == 8) {
                return PixelFormatFromString("GRAY8");
            }else if(depth == 16) {
                return PixelFormatFromString("Y400A");
            }
        }
    }
    throw std::runtime_error("Unsupported TGA format");
}

TypedImage LoadTga(const std::string& filename)
{
    FILE *file;
    unsigned char type[4];
    unsigned char info[6];

    file = fopen(filename.c_str(), "rb");

    if(file) {
        bool success = true;
        success &= fread( &type, sizeof (char), 3, file ) == 3;
        fseek( file, 12, SEEK_SET );
        success &= fread( &info, sizeof (char), 6, file ) == 6;

        const int width  = info[0] + (info[1] * 256);
        const int height = info[2] + (info[3] * 256);

        if(success) {
            TypedImage img(width, height, TgaFormat(info[4], type[2], type[1]) );

            //read in image data
            const size_t data_size = img.h * img.pitch;
            success &= fread(img.ptr, sizeof(unsigned char), data_size, file) == data_size;
            fclose(file);
            return img;
        }else{
            fclose(file);
        }
    }

    throw std::runtime_error("Unable to load TGA file, '" + filename + "'");
}

}
