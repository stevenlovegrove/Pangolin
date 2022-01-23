#include <fstream>
#include <pangolin/image/typed_image.h>

#ifdef HAVE_LIBRAW
#  include <libraw/libraw.h>
#endif

namespace pangolin {

TypedImage LoadLibRaw(
    const std::string& filename
) {
#ifdef HAVE_LIBRAW
    static LibRaw RawProcessor;

    int ret;

    if ((ret = RawProcessor.open_file(filename.c_str())) != LIBRAW_SUCCESS)
    {
        throw std::runtime_error(libraw_strerror(ret));
    }

    if ((ret = RawProcessor.unpack()) != LIBRAW_SUCCESS)
    {
        throw std::runtime_error(libraw_strerror(ret));
    }

    const auto& S = RawProcessor.imgdata.sizes;
    TypedImage image(S.width, S.height, PixelFormatFromString("GRAY16LE"), sizeof(uint16_t) * S.raw_width);
    PitchedCopy((char*)image.ptr, image.pitch, (char*)RawProcessor.imgdata.rawdata.raw_image, sizeof(uint16_t) * S.raw_width, sizeof(uint16_t) * image.w, image.h);

    // TODO: Support image metadata so that we can extract these fields.
    // RawProcessor.imgdata.other.iso_speed
    // RawProcessor.imgdata.other.aperture
    // RawProcessor.imgdata.other.focal_len
    // RawProcessor.imgdata.other.shutter
    // RawProcessor.imgdata.other.timestamp
    // RawProcessor.imgdata.makernotes.common.exifCameraElevationAngle
    // RawProcessor.imgdata.makernotes.sony.SonyDateTime

    return image;
#else
    PANGOLIN_UNUSED(filename);
    throw std::runtime_error("Rebuild Pangolin for libraw support.");
#endif
}

}
