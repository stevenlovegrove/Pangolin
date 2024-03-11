#pragma once

#include <pangolin/image/image_io.h>

#include <memory>

namespace pangolin
{

using ImageEncoderFunc =
    std::function<void(std::ostream&, const sophus2::IntensityImage<>&)>;
using ImageDecoderFunc = std::function<sophus2::IntensityImage<>(std::istream&)>;

class StreamEncoderFactory
{
  public:
  static StreamEncoderFactory& I();

  ImageEncoderFunc GetEncoder(
      const std::string& encoder_spec, const sophus2::PixelFormat& fmt);

  ImageDecoderFunc GetDecoder(
      const std::string& encoder_spec, const sophus2::PixelFormat& fmt);
};

}  // namespace pangolin
