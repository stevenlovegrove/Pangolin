#include <fstream>
#include <pangolin/image/typed_image.h>

namespace pangolin {

#pragma pack(push, 1)
struct BitmapHeader
{
  uint16_t magic;
  uint32_t filesize_bytes;
  uint32_t reserved;
  uint32_t offset_bytes;  
};
struct BitmapInfoHeader
{
  uint32_t header_size_bytes;
  uint32_t width;
  uint32_t height;
  uint16_t color_planes;
  uint16_t bits_per_pixel;
  uint32_t compression_method;
  uint32_t imagesize_bytes;
  uint32_t w_pixel_per_meter;
  uint32_t h_pixel_per_meter;
  uint32_t colors_in_pallete;
  uint32_t important_colors;
};

static_assert(sizeof(BitmapHeader) == 14, "Unexpected padding on struct");
static_assert(sizeof(BitmapInfoHeader) == 40, "Unexpected padding on struct");
#pragma pack(pop)


// https://en.wikipedia.org/wiki/BMP_file_format
TypedImage LoadBmp(std::istream& in)
{
  constexpr uint16_t expected_magic = 'B' | 'M' << 8;

  BitmapHeader bmp_file_header;
  BitmapInfoHeader bmp_info_header;

  memset((char*)&bmp_file_header, 0, sizeof(bmp_file_header));
  memset((char*)&bmp_info_header, 0, sizeof(bmp_info_header));

  in.read((char*)&bmp_file_header, sizeof(bmp_file_header));
  if(!in.good() || bmp_file_header.magic != expected_magic) 
    throw std::runtime_error("LoadBmp: invalid magic header");

  in.read((char*)&bmp_info_header, sizeof(bmp_info_header));
  if(!in.good() || bmp_info_header.header_size_bytes != 40)
    throw std::runtime_error("LoadBmp: unknown info header");

  if(bmp_info_header.bits_per_pixel != 24 )
    throw std::runtime_error("LoadBmp: unexpected format");

  const PixelFormat fmt = PixelFormatFromString("RGB24");

  const size_t w = bmp_info_header.width;
  const size_t h = bmp_info_header.height;
  const size_t padding_bytes = ((4 - (w * 3) % 4) % 4);

  if( w == 0 || h == 0)
    throw std::runtime_error("LoadBmp: Invalid Bitmap size");

  TypedImage img(w, h, fmt);

  for (int y = ( (int)h - 1); y != -1; y--)
  {
    char* p_pix = (char*)img.RowPtr(y);
    in.read(p_pix, w * fmt.channels);
    
    if(!in.good()) 
      throw std::runtime_error("LoadBmp: Unexpected end of stream.");

    // Convert from BGR to RGB
    for (size_t x = 0; x < w; x++)
    {
      // BGR -> RGB
      std::swap(p_pix[0], p_pix[2]);
      p_pix += fmt.channels;
    }

    in.ignore(padding_bytes);
  }

  return img;
}

void SaveBmp(const Image<unsigned char>& /*image*/, const pangolin::PixelFormat& /*fmt*/, std::ostream& /*out*/, bool /*top_line_first*/)
{
  throw std::runtime_error("SaveBMP: Not implemented");
}

}
