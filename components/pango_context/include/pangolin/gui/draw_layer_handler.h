#pragma once

#include <pangolin/render/depth_sampler.h>
#include <pangolin/gui/interactive.h>
#include <pangolin/utils/signal_slot.h>
#include <sophus/sensor/camera_model.h>

namespace pangolin {

struct DrawLayer;
struct DrawLayerRenderState;

enum class ViewMode {
  freeview,
  image_plane,
  best_guess
};

struct SelectionEvent {
    Interactive::Event trigger_event;
    MinMax<Eigen::Array2d> in_pixel_selection;
    bool in_progress;
};

struct DrawLayerHandler {
 public:

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
    const Eigen::Array2d& pos_clip,
    const Eigen::Array2d& pos_img,
    Eigen::Array2d clip_aspect_scale,
    DrawLayer& layer,
    DrawLayerRenderState& render_state
  ) = 0;

  virtual ViewMode viewMode() = 0;
  virtual void setViewMode(ViewMode view_mode) = 0;

  sigslot::signal<SelectionEvent> selection_signal;

  struct Params {
    Shared<DepthSampler> depth_sampler = DepthSampler::Create({});
    Eigen::Vector3d up_in_world = {0.0, 0.0, 1.0};
    MinMax<Eigen::Vector3d> camera_limits_in_world = {};
    ViewMode view_mode = ViewMode::best_guess;
  };
  static Shared<DrawLayerHandler> Create(Params const &);
};

}  // namespace pangolin
