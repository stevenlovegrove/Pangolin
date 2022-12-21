#include <pangolin/image/runtime_image.h>

#include <algorithm>
#include <fstream>
#include <memory>

namespace pangolin
{

#pragma pack(push, 1)
struct packed12bit_image_header {
  char magic[4];
  char fmt[16];
  size_t w, h;
};
#pragma pack(pop)

void SavePacked12bit(IntensityImage<> const& image, std::ostream& out)
{
  if (image.numBytesPerPixelChannel() != 2) {
    throw std::runtime_error(
        "packed12bit currently only supported with 16bit input image");
  }

  const size_t dest_pitch =
      (image.width() * 12) / 8 + ((image.width() * 12) % 8 > 0 ? 1 : 0);
  const size_t dest_size = image.height() * dest_pitch;
  std::unique_ptr<uint8_t[]> output_buffer(new uint8_t[dest_size]);

  for (int r = 0; r < image.height(); ++r) {
    uint8_t* pout = output_buffer.get() + r * dest_pitch;
    uint16_t* pin = (uint16_t*)(image.rawRowPtr(r));
    uint16_t const* pin_end = (uint16_t*)(image.rawRowPtr(r + 1));
    while (pin < pin_end) {
      uint32_t val = (*(pin++) & 0x00000FFF);
      val |= uint32_t(*(pin++) & 0x00000FFF) << 12;
      *(pout++) = uint8_t(val & 0x000000FF);
      *(pout++) = uint8_t((val & 0x0000FF00) >> 8);
      *(pout++) = uint8_t((val & 0x00FF0000) >> 16);
    }
  }

  packed12bit_image_header header;
  static_assert(sizeof(header.magic) == 4, "[bug]");
  const std::string fmt_str = ToString(image.pixelType());
  memcpy(header.magic, "P12B", 4);
  memset(header.fmt, '\0', sizeof(header.fmt));
  memcpy(
      header.fmt, fmt_str.c_str(),
      std::min(sizeof(header.fmt), fmt_str.size()));
  header.w = image.width();
  header.h = image.height();
  out.write((char*)&header, sizeof(header));
  out.write((char*)output_buffer.get(), dest_size);
}

IntensityImage<> LoadPacked12bit(std::istream& in)
{
  // Read in header, uncompressed
  packed12bit_image_header header;
  in.read((char*)&header, sizeof(header));

  IntensityImage<> img(
      sophus::ImageSize(header.w, header.h), PixelFormatFromString(header.fmt));

  if (img.numBytesPerPixelChannel() != 2) {
    throw std::runtime_error(
        "packed12bit currently only supported with 16bit input image");
  }

  const size_t input_pitch =
      (img.width() * 12) / 8 + ((img.width() * 12) % 8 > 0 ? 1 : 0);
  const size_t input_size = img.height() * input_pitch;
  std::unique_ptr<uint8_t[]> input_buffer(new uint8_t[input_size]);

  in.read((char*)input_buffer.get(), input_size);

  for (int r = 0; r < img.height(); ++r) {
    uint16_t* pout = (uint16_t*)(img.rawRowPtr(r));
    uint8_t* pin = input_buffer.get() + r * input_pitch;
    uint8_t const* pin_end = input_buffer.get() + (r + 1) * input_pitch;
    while (pin < pin_end) {
      uint32_t val = *(pin++);
      val |= uint32_t(*(pin++)) << 8;
      val |= uint32_t(*(pin++)) << 16;
      *(pout++) = uint16_t(val & 0x000FFF);
      *(pout++) = uint16_t((val & 0xFFF000) >> 12);
    }
  }

  return img;
}

}  // namespace pangolin
