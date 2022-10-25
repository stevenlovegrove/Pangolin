#pragma once

#include <optional>
#include <Eigen/Core>
#include <farm_ng/core/logging/expected.h>
#include <sophus/image/image.h>
#include <pangolin/maths/min_max.h>

namespace pangolin {

class FramebufferReader {
 public:
  enum class DepthKind { clip, zaxis, radial };

  virtual ~FramebufferReader() {}

  virtual farm_ng::Expected<MinMax<double>> getDepth(
      Eigen::Vector2i const& pix,
      int patch_rad,
      DepthKind = DepthKind::zaxis) const = 0;
};

}
