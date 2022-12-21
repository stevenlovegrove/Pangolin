#pragma once

#include <pangolin/render/depth_sampler.h>

namespace pangolin
{

class DepthSamplerImage : public DepthSampler
{
  public:
  virtual void setDepthImage(sophus::Image<float> const& image) = 0;

  struct Params {
    DepthKind kind = DepthKind::zaxis;
    sophus::Image<float> depth_image;
  };
  static Shared<DepthSamplerImage> Create(Params p);
};

}  // namespace pangolin
