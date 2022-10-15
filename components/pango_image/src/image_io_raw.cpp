#include <fstream>
#include <pangolin/image/typed_image.h>

namespace pangolin {

TypedImage LoadImageNonPlanar(
    const std::string& filename,
    const PixelFormat& raw_fmt, size_t raw_width, size_t raw_height,
    size_t raw_pitch, size_t offset
) {
    TypedImage img(raw_width, raw_height, raw_fmt, raw_pitch);

    // Read from file, row at a time.
    std::ifstream bFile( filename.c_str(), std::ios::in | std::ios::binary );
    bFile.seekg(offset);
    for(size_t r=0; r<img.h; ++r) {
        bFile.read( (char*)img.ptr + r*img.pitch, img.pitch );
        if(bFile.fail()) {
            pango_print_warn("Unable to read raw image file to completion.");
            break;
        }
    }
    return img;
}

template<typename Tin, typename Tout>
TypedImage ToNonPlanarT(const TypedImage& planar_image, const PixelFormat& new_fmt)
{
    const size_t planes = new_fmt.channels;

    PANGO_ENSURE(planar_image.h % planes == 0);
    PANGO_ENSURE(sizeof(Tin)*planes == sizeof(Tout));
    PANGO_ENSURE(sizeof(Tout) == new_fmt.bpp / 8);
    PANGO_ENSURE(new_fmt.planar == false);

    TypedImage image(planar_image.w, planar_image.h / planes, new_fmt);

    Image<Tin> in = planar_image.UnsafeReinterpret<Tin>();
    Image<Tout> out = image.UnsafeReinterpret<Tout>();

    for(size_t c=0; c < planes; ++c) {
        Image<Tin> in_plane = in.SubImage(0, out.h*c, out.w, out.h);

        for(size_t y=0; y < image.h; ++y)
        {
            Tin* p_out = (Tin*)out.RowPtr(y) + c;
            Tin* p_in  = in_plane.RowPtr(y);
            Tin* p_end  = p_in + in_plane.w;

            while(p_in != p_end) {
                *p_out = *p_in;

                ++p_in;
                p_out += planes;
            }
        }
    }

    return image;
}

TypedImage ToNonPlanar(const TypedImage& planar, size_t planes)
{
    if(planes == 3) {
        if(planar.fmt.format == "GRAY8") {
            return ToNonPlanarT<uint8_t,Eigen::Matrix<uint8_t,3,1>>(planar, PixelFormatFromString("RGB24"));
        }else if(planar.fmt.format == "GRAY16LE") {
            return ToNonPlanarT<uint8_t,Eigen::Matrix<uint16_t,3,1>>(planar, PixelFormatFromString("RGB48"));
        }else if(planar.fmt.format == "GRAY32F") {
            return ToNonPlanarT<uint8_t,Eigen::Matrix<float,3,1>>(planar, PixelFormatFromString("RGB96F"));
        }
    }else if(planes == 4) {
        if(planar.fmt.format == "GRAY8") {
            return ToNonPlanarT<uint8_t,Eigen::Matrix<uint8_t,4,1>>(planar, PixelFormatFromString("RGBA32"));
        }else if(planar.fmt.format == "GRAY16LE") {
            return ToNonPlanarT<uint8_t,Eigen::Matrix<uint16_t,4,1>>(planar, PixelFormatFromString("RGBA64"));
        }else if(planar.fmt.format == "GRAY32F") {
            return ToNonPlanarT<uint8_t,Eigen::Matrix<float,4,1>>(planar, PixelFormatFromString("RGBA128F"));
        }
    }

    throw std::runtime_error("Unable to convert planar image of type " + planar.fmt.format);
}

TypedImage LoadImage(
    const std::string& filename,
    const PixelFormat& raw_plane_fmt, size_t raw_width, size_t raw_height,
    size_t raw_pitch, size_t offset, size_t image_planes
) {
    image_planes = std::max<size_t>(1,image_planes);

    if(image_planes > 1) {
        // Load as large image
        TypedImage planar = LoadImageNonPlanar(filename, raw_plane_fmt, raw_width, raw_height*image_planes, raw_pitch, offset);
        planar.fmt.planar = true;

        // Convert into non-planar image
        return ToNonPlanar(planar, image_planes);

    }else{
        return LoadImageNonPlanar(filename, raw_plane_fmt, raw_width, raw_height, raw_pitch, offset);
    }
}

}
