#pragma once

#include <pangolin/render/depth_sampler.h>
#include <pangolin/gui/interactive.h>
#include <pangolin/gui/draw_layer.h>
#include <pangolin/utils/signal_slot.h>
#include <sophus/sensor/camera_model.h>

namespace pangolin {

struct DrawLayerHandler {
 public:
  virtual ~DrawLayerHandler() {}

  // Handle 2D window events
  virtual bool handleEvent(
    const Context& context,
    const Interactive::Event& event,
    DrawLayer& layer
  ) = 0;

  struct Params {
    std::shared_ptr<DepthSampler> depth_sampler = DepthSampler::Create({});
    Eigen::Vector3d up_in_world = {0.0, 0.0, 1.0};
    MinMax<Eigen::Vector3d> camera_limits_in_world = {};
    bool camera_rotation_lock = false;
  };
  static std::unique_ptr<DrawLayerHandler> Create(Params const &);
};

}  // namespace pangolin
