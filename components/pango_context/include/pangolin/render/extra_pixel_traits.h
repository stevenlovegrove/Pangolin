#pragma once

#include <pangolin/gl/color.h>
#include <sophus/image/image_types.h>
#include <sophus/lie/se3.h>

// Specialization of sophus::ImageTraits for types we might like to consider
// in pixel or vertex-like buffers

namespace sophus
{

template <class TT>
struct ImageTraits<So3<TT>> {
  // (X,Y,Z,W) quaternion
  static int const kNumChannels = 4;
  using TPixel = So3<TT>;
  using ChannelT = TT;
};

template <class TT>
struct ImageTraits<Se3<TT>> {
  // (X,Y,Z,W) quaternion +
  // (tx,ty,tz,_) translation
  // where _ is padding with undefined value
  static int const kNumChannels = 8;
  using TPixel = Se3<TT>;
  using ChannelT = TT;
};

template <>
struct ImageTraits<pangolin::Color> {
  static int const kNumChannels = 4;
  using TPixel = pangolin::Color;
  using ChannelT = float;
};

}  // namespace sophus
