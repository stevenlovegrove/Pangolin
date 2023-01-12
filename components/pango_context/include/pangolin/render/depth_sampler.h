#pragma once

#include <Eigen/Core>
#include <pangolin/maths/region.h>
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
    RegionF64 min_max = RegionF64::empty();
    DepthKind depth_kind;
  };

  struct SampleLocation {
    Eigen::Array2d pos_camera_pixel;
    Eigen::Array2d pos_window;
  };

  virtual std::optional<Sample> sampleDepth(
      const SampleLocation& pix, int patch_rad, RegionF64 near_far,
      const Context* default_context) = 0;

  virtual ~DepthSampler() {}

  struct Params {
    std::shared_ptr<Context> context;
  };
  static Shared<DepthSampler> Create(Params p);
};

}  // namespace pangolin
