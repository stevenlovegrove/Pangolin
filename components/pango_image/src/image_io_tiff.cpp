#include <pangolin/image/image_io.h>

#include <cassert>

#ifdef HAVE_LIBTIFF
#include <tiffio.h>
#endif

using namespace sophus;

namespace pangolin
{

#ifdef HAVE_LIBTIFF
template <typename T>
T GetOrThrow(TIFF* tif, uint32_t tag)
{
  T r;
  if (TIFFGetField(tif, tag, &r) != 1) {
    throw std::runtime_error(
        "Expected tag missing when reading tiff (" + std::to_string(tag) + ")");
  }
  return r;
}

template <typename T>
T GetOrDefault(TIFF* tif, uint32_t tag, T default_val)
{
  T r = default_val;
  TIFFGetField(tif, tag, &r);
  return r;
}

template <typename T>
void SetOrThrow(TIFF* tif, uint32_t tag, T val)
{
  if (TIFFSetField(tif, tag, val) != 1) {
    throw std::runtime_error(
        "Error when writing tiff tag (" + std::to_string(tag) + ")");
  }
}

#endif

void DummyTiffOpenHandler(const char* module, const char* fmt, va_list ap)
{
  // TODO: Should probably send these somewhere...
}

void SaveTiff(const IntensityImage<>& image, const std::string& filename)
{
#ifdef HAVE_LIBTIFF

  TIFFSetWarningHandler(DummyTiffOpenHandler);

  TIFF* tif = TIFFOpen(filename.c_str(), "w");
  if (!tif) {
    throw std::runtime_error("libtiff failed to open " + filename);
  }

  SetOrThrow<uint32_t>(tif, TIFFTAG_IMAGEWIDTH, image.width());
  SetOrThrow<uint32_t>(tif, TIFFTAG_IMAGELENGTH, image.height());
  SetOrThrow<uint16_t>(tif, TIFFTAG_SAMPLESPERPIXEL, image.numChannels());
  SetOrThrow<uint16_t>(
      tif, TIFFTAG_BITSPERSAMPLE,
      image.pixelFormat().num_bytes_per_component * 8);
  if (image.pixelFormat().is<uint8_t>() || image.pixelFormat().is<Pixel3U8>()) {
    SetOrThrow<uint16_t>(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
  } else if (
      image.pixelFormat().is<float>() || image.pixelFormat().is<Pixel3F32>()) {
    SetOrThrow<uint16_t>(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
  } else {
    throw std::runtime_error(
        "TIFF support is currently limited. Consider contributing to "
        "image_io_tiff.cpp.");
  }
  SetOrThrow<uint16_t>(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

  for (uint32_t row = 0; row < static_cast<uint32_t>(image.height()); ++row) {
    auto result =
        TIFFWriteScanline(tif, const_cast<uint8_t*>(image.rawRowPtr(row)), row);
    if (result != 1) {
      FARM_WARN("TIFFWriteScanline error");
    }
  }

  TIFFClose(tif);

#else
  PANGOLIN_UNUSED(filename);
  throw std::runtime_error("Rebuild Pangolin for libtiff support.");
#endif
}

IntensityImage<> LoadTiff(const std::string& filename)
{
#ifdef HAVE_LIBTIFF
  TIFFSetWarningHandler(DummyTiffOpenHandler);

  TIFF* tif = TIFFOpen(filename.c_str(), "r");
  if (!tif) {
    throw std::runtime_error("libtiff failed to open " + filename);
  }

  const auto width = GetOrThrow<uint32_t>(tif, TIFFTAG_IMAGEWIDTH);
  const auto height = GetOrThrow<uint32_t>(tif, TIFFTAG_IMAGELENGTH);
  const auto channels = GetOrThrow<uint16_t>(tif, TIFFTAG_SAMPLESPERPIXEL);
  const auto bits_per_channel =
      GetOrThrow<uint16_t>(tif, TIFFTAG_BITSPERSAMPLE);
  const auto sample_format =
      GetOrDefault<uint16_t>(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
  const auto planar = GetOrThrow<uint16_t>(tif, TIFFTAG_PLANARCONFIG);
  //    const auto photom = GetOrThrow<uint16_t>(tif, TIFFTAG_PHOTOMETRIC);

  // comparison of unsigned with >= 0 is always true!
  // assert(width >= 0 && height >= 0 && channels >= 0 && bits_per_channel > 0);

  if (planar != PLANARCONFIG_CONTIG /*|| photom != PHOTOMETRIC_RGB*/ ||
      bits_per_channel % 8 != 0 || !(channels == 1 || channels == 3))
    throw std::runtime_error(
        "TIFF support is currently limited. Consider contributing to "
        "image_io_tiff.cpp.");

  std::string sfmt;
  switch (sample_format) {
    case SAMPLEFORMAT_UINT:
      sfmt = (channels == 3) ? "RGB24" : "GRAY8";
      break;
    case SAMPLEFORMAT_IEEEFP:
      sfmt = (channels == 3) ? "RGB96F" : "GRAY32F";
      break;
    default:
      throw std::runtime_error(
          "TIFF support is currently limited. Consider contributing to "
          "image_io_tiff.cpp.");
  }

  auto image = IntensityImage<>::fromFormat(
      sophus::ImageSize(width, height), PixelFormatFromString(sfmt.c_str()));
  const tsize_t scanlength_bytes = TIFFScanlineSize(tif);
  if (scanlength_bytes != tsize_t(image.pitchBytes()))
    throw std::runtime_error("TIFF: unexpected scanline length");

  for (size_t row = 0; row < height; ++row) {
    TIFFReadScanline(tif, const_cast<uint8_t*>(image.rawRowPtr(row)), row);
  }

  TIFFClose(tif);

  return image;
#else
  PANGOLIN_UNUSED(filename);
  throw std::runtime_error("Rebuild Pangolin for libtiff support.");
#endif
}

}  // namespace pangolin
