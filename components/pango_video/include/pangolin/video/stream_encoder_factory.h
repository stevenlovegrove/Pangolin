#pragma once

#include <pangolin/image/image_io.h>

#include <memory>

namespace pangolin
{

using ImageEncoderFunc =
    std::function<void(std::ostream&, const sophus::IntensityImage<>&)>;
using ImageDecoderFunc = std::function<sophus::IntensityImage<>(std::istream&)>;

class StreamEncoderFactory
{
  public:
  static StreamEncoderFactory& I();

  ImageEncoderFunc GetEncoder(
      const std::string& encoder_spec, const RuntimePixelType& fmt);

  ImageDecoderFunc GetDecoder(
      const std::string& encoder_spec, const RuntimePixelType& fmt);
};

}  // namespace pangolin
