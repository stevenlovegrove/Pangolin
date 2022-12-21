#pragma once

#include <pangolin/image/image_io.h>

#include <memory>

namespace pangolin
{

using ImageEncoderFunc =
    std::function<void(std::ostream&, sophus::IntensityImage<> const&)>;
using ImageDecoderFunc = std::function<sophus::IntensityImage<>(std::istream&)>;

class StreamEncoderFactory
{
  public:
  static StreamEncoderFactory& I();

  ImageEncoderFunc GetEncoder(
      std::string const& encoder_spec, RuntimePixelType const& fmt);

  ImageDecoderFunc GetDecoder(
      std::string const& encoder_spec, RuntimePixelType const& fmt);
};

}  // namespace pangolin
