#pragma once

#include <pangolin/render/depth_sampler.h>
#include <pangolin/handler/handler.h>
#include <pangolin/handler/interactive.h>
#include <pangolin/utils/signal_slot.h>
#include <sophus/sensor/camera_model.h>

namespace pangolin {

struct DrawLayer;

struct Handler {
 public:
  virtual ~Handler() {}

  // Handle 2D window events
  virtual bool handleEvent(
    DrawLayer& layer,
    sophus::Se3F64& camera_from_world_,
    sophus::CameraModel& camera,
    MinMax<double>& near_far,
    const Context& context,
    const Interactive::Event& event
  ) = 0;

  // Pointer / touch state
  struct Pointer {
    Eigen::Vector2d p_img;
    Eigen::Vector3d p_cam;
    sophus::Se3F64 cam_T_world;
    double z_depth_cam;
  };

  // Processed event the Handler can generate
  struct Event3D {
    Interactive::Event trigger;
    Pointer pointer;
  };

  virtual MinMax<Eigen::Vector3d> getCameraLimits() const = 0;
  virtual void setCameraLimits(const MinMax<Eigen::Vector3d>& limits_in_world) = 0;
  virtual bool getCameraRotationLock() const = 0;
  virtual void setCameraRotationLock(bool enable = true) = 0;

  sigslot::signal<Event3D> InputSignal;

  //------------------------------------------------------------
  // Factory construction

  struct Params {
    std::shared_ptr<DepthSampler> depth_sampler = DepthSampler::Create({});
    Eigen::Vector3d up_in_world = {0.0, 0.0, 1.0};
    MinMax<Eigen::Vector3d> camera_limits_in_world = {};
    bool camera_rotation_lock = false;
  };
  static std::unique_ptr<Handler> Create(Params const &);
};

}  // namespace pangolin
