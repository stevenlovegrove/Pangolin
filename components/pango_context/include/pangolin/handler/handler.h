#pragma once

#include <pangolin/render/depth_sampler.h>
#include <pangolin/handler/handler.h>
#include <pangolin/handler/interactive.h>
#include <sophus/sensor/camera_model.h>

namespace pangolin {

struct DrawLayer;

struct Handler {
 public:
  virtual ~Handler() {}

  virtual bool handleEvent(
    DrawLayer& layer,
    sophus::Se3F64& camera_from_world_,
    sophus::CameraModel& camera,
    MinMax<double>& near_far,
    MinMax<Eigen::Vector3d>& camera_limits_in_world,
    const Context& context,
    const Interactive::Event& event
  ) = 0;

  struct Params {
    std::shared_ptr<DepthSampler> depth_sampler = DepthSampler::Create({});
  };
  static std::unique_ptr<Handler> Create(Params const &);
};

}  // namespace pangolin
