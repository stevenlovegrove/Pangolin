#pragma once

#include <optional>
#include <Eigen/Core>
#include <farm_ng/core/logging/expected.h>
#include <sophus/image/image.h>
#include <pangolin/maths/min_max.h>
#include <pangolin/utils/shared.h>

namespace pangolin {

struct Context;

class DepthSampler {
 public:
  enum class DepthKind { clip, zaxis, radial };

  struct Sample {
    MinMax<double> min_max;
    DepthKind depth_kind;
  };

  virtual std::optional<Sample> sampleDepth(
    const Eigen::Array2i& pix, int patch_rad,
    MinMax<double> near_far, const Context* default_context
  ) = 0;

  virtual ~DepthSampler() {}

  struct Params{
    std::shared_ptr<Context> context;
  };
  static Shared<DepthSampler> Create(Params p);
};

}
