#include <cassert>
#include <pangolin/image/typed_image.h>

#ifdef HAVE_LIBTIFF
#  include <tiffio.h>
#endif

namespace pangolin {

#ifdef HAVE_LIBTIFF
template<typename T>
T GetOrThrow(TIFF* tif, uint32_t tag)
{
    T r;
    if (TIFFGetField(tif, tag, &r) != 1) {
        throw std::runtime_error("Expected tag missing when reading tiff (" + std::to_string(tag) + ")");
    }
    return r;
}

template<typename T>
T GetOrDefault(TIFF* tif, uint32_t tag, T default_val)
{
    T r = default_val;
    TIFFGetField(tif, tag, &r);
    return r;
}
#endif

void DummyTiffOpenHandler(const char* module, const char* fmt, va_list ap)
{
    // TODO: Should probably send these somewhere...
}

TypedImage LoadTiff(
    const std::string& filename
) {
#ifdef HAVE_LIBTIFF
    TIFFSetWarningHandler(DummyTiffOpenHandler);

    TIFF* tif = TIFFOpen(filename.c_str(),"r");
    if (!tif) {
        throw std::runtime_error("libtiff failed to open " + filename);
    }

    const auto width = GetOrThrow<uint32_t>(tif, TIFFTAG_IMAGEWIDTH);
    const auto height = GetOrThrow<uint32_t>(tif, TIFFTAG_IMAGELENGTH);
    const auto channels = GetOrThrow<uint16_t>(tif, TIFFTAG_SAMPLESPERPIXEL);
    const auto bits_per_channel = GetOrThrow<uint16_t>(tif, TIFFTAG_BITSPERSAMPLE);
    const auto sample_format = GetOrDefault<uint16_t>(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
    const auto planar = GetOrThrow<uint16_t>(tif, TIFFTAG_PLANARCONFIG);
//    const auto photom = GetOrThrow<uint16_t>(tif, TIFFTAG_PHOTOMETRIC);

    // comparison of unsigned with >= 0 is always true!
    //assert(width >= 0 && height >= 0 && channels >= 0 && bits_per_channel > 0);

    if(planar != PLANARCONFIG_CONTIG /*|| photom != PHOTOMETRIC_RGB*/ || bits_per_channel % 8 != 0 || !(channels == 1 || channels == 3))
        throw std::runtime_error("TIFF support is currently limited. Consider contributing to image_io_tiff.cpp.");

    std::string sfmt;
    switch(sample_format) {
    case SAMPLEFORMAT_UINT: sfmt = (channels == 3) ? "RGB24" : "GRAY8"; break;
    case SAMPLEFORMAT_IEEEFP: sfmt = (channels == 3) ? "RGB96F" : "GRAY32F"; break;
    default: throw std::runtime_error("TIFF support is currently limited. Consider contributing to image_io_tiff.cpp.");
    }

    TypedImage image(width, height, PixelFormatFromString(sfmt));
    const tsize_t scanlength_bytes = TIFFScanlineSize(tif);
    if(scanlength_bytes != tsize_t(image.pitch))
        throw std::runtime_error("TIFF: unexpected scanline length");

    for (size_t row = 0; row < height; ++row) {
        TIFFReadScanline(tif, image.RowPtr(row), row);
    }

    TIFFClose(tif);

    return image;
#else
    PANGOLIN_UNUSED(filename);
    throw std::runtime_error("Rebuild Pangolin for libtiff support.");
#endif
}

}

