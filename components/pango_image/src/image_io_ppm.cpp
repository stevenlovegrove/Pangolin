#include <pangolin/image/runtime_image.h>

#include <fstream>

namespace pangolin
{

RuntimePixelType PpmFormat(const std::string& strType, int num_colours)
{
  if (strType == "P5") {
    if (num_colours < 256) {
      return PixelFormatFromString("GRAY8");
    } else {
      return PixelFormatFromString("GRAY16LE");
    }
  } else if (strType == "P6") {
    return PixelFormatFromString("RGB24");
  } else {
    throw std::runtime_error("Unsupported PPM/PGM format");
  }
}

void PpmConsumeWhitespaceAndComments(std::istream& in)
{
  // TODO: Make a little more general / more efficient
  while (in.peek() == ' ') in.get();
  while (in.peek() == '\n') in.get();
  while (in.peek() == '#') in.ignore(4096, '\n');
}

IntensityImage<> LoadPpm(std::istream& in)
{
  // Parse header
  std::string ppm_type = "";
  int num_colors = 0;
  int w = 0;
  int h = 0;

  in >> ppm_type;
  PpmConsumeWhitespaceAndComments(in);
  in >> w;
  PpmConsumeWhitespaceAndComments(in);
  in >> h;
  PpmConsumeWhitespaceAndComments(in);
  in >> num_colors;
  in.ignore(1, '\n');

  if (!in.fail() && w > 0 && h > 0) {
    IntensityImage<> img(
        sophus::ImageSize(w, h), PpmFormat(ppm_type, num_colors));

    // Read in data
    for (int r = 0; r < img.height(); ++r) {
      in.read((char*)img.rawRowPtr(r), img.pitchBytes());
    }
    if (!in.fail()) {
      return img;
    }
  }

  throw std::runtime_error("Unable to load PPM file.");
}

void SavePpm(
    const IntensityImage<>& image, std::ostream& out, bool top_line_first)
{
  using namespace sophus;

  // Setup header variables
  std::string ppm_type = "";
  const int w = (int)image.width();
  const int h = (int)image.height();

  int num_colors = 0;

  if (image.pixelType().is<uint8_t>()) {
    ppm_type = "P5";
    num_colors = 255;
  } else if (image.pixelType().is<uint16_t>()) {
    ppm_type = "P5";
    num_colors = 65535;
  } else if (image.pixelType().is<Pixel3U8>()) {
    ppm_type = "P6";
    num_colors = 255;
  } else {
    throw std::runtime_error("Unsupported pixel format");
  }

  // Write header
  out << ppm_type;
  out << " ";
  out << w;
  out << " ";
  out << h;
  out << " ";
  out << num_colors;
  out << "\n";

  // Write out data
  if (top_line_first) {
    for (int r = 0; r < h; ++r) {
      out.write((char*)image.rawRowPtr(r), image.pitchBytes());
    }
  } else {
    for (int r = 0; r < h; ++r) {
      out.write((char*)image.rawRowPtr(h - 1 - r), image.pitchBytes());
    }
  }
}

}  // namespace pangolin
