#pragma once

#include <pangolin/render/depth_sampler.h>
#include <pangolin/gui/interactive.h>
#include <pangolin/gui/draw_layer.h>
#include <pangolin/utils/signal_slot.h>
#include <sophus/sensor/camera_model.h>

namespace pangolin {

struct DrawLayerHandler {
 public:
  enum class ViewMode {
    freeview,
    image_plane
  };

  // enum class CursorUpdate {
  //   with_click,        // Cursor of rotation is updated with every mouse-down event
  //   with_double_click, // Cursor of rotation is updated with every double-click to form
  //                      // a kind of 3D cursor
  //   fixed,             // e.g. programmatic object-oriented navigation
  // };

  virtual ~DrawLayerHandler() {}

  // Handle 2D window events
  virtual bool handleEvent(
    const Context& context,
    const Interactive::Event& event,
    Eigen::Matrix3d clip_from_window,
    Eigen::Matrix3d pixel_from_window,
    Eigen::Array2d clip_aspect_scale,
    DrawLayer& layer,
    DrawLayer::RenderState& render_state
  ) = 0;

  virtual void setViewMode(ViewMode view_mode) = 0;

  struct Params {
    std::shared_ptr<DepthSampler> depth_sampler = DepthSampler::Create({});
    Eigen::Vector3d up_in_world = {0.0, 0.0, 1.0};
    MinMax<Eigen::Vector3d> camera_limits_in_world = {};
    ViewMode view_mode = ViewMode::freeview;
  };
  static std::unique_ptr<DrawLayerHandler> Create(Params const &);
};

}  // namespace pangolin
