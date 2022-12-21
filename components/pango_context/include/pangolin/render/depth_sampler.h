#pragma once

#include <Eigen/Core>
#include <farm_ng/core/logging/expected.h>
#include <pangolin/maths/min_max.h>
#include <pangolin/utils/shared.h>
#include <sophus/image/image.h>

#include <optional>

namespace pangolin
{

struct Context;

class DepthSampler
{
  public:
  enum class DepthKind { clip, zaxis, radial };

  struct Sample {
    MinMax<double> min_max;
    DepthKind depth_kind;
  };

  struct SampleLocation {
    Eigen::Array2d pos_camera_pixel;
    Eigen::Array2d pos_window;
  };

  virtual std::optional<Sample> sampleDepth(
      SampleLocation const& pix, int patch_rad, MinMax<double> near_far,
      Context const* default_context) = 0;

  virtual ~DepthSampler() {}

  struct Params {
    std::shared_ptr<Context> context;
  };
  static Shared<DepthSampler> Create(Params p);
};

}  // namespace pangolin
