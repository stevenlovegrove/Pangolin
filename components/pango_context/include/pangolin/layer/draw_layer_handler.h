#pragma once

#include <pangolin/context/depth_sampler.h>
#include <pangolin/layer/interactive.h>
#include <pangolin/render/conventions.h>
#include <sigslot/signal.hpp>
#include <sophus2/sensor/camera_model.h>

namespace pangolin
{

struct DrawLayer;
struct DrawLayerRenderState;

enum class ViewMode { freeview, image_plane, best_guess };

struct SelectionEvent {
  Interactive::Event trigger_event;
  Interactive::PointerEvent pointer_event;
  sophus2::Region2F64 in_pixel_selection = sophus2::Region2F64::empty();
  Eigen::Vector3d in_scene_cursor;
  Eigen::Vector3d in_scene_hover;
  bool in_progress;
};

struct DrawLayerHandler {
  public:
  virtual ~DrawLayerHandler() {}

  // Handle 2D window events
  virtual bool handleEvent(
      const Context& context, const Interactive::Event& event,
      const Eigen::Array2d& pos_clip, const Eigen::Array2d& pos_img,
      Eigen::Array2d clip_aspect_scale, DrawLayer& layer,
      DrawLayerRenderState& render_state) = 0;

  virtual ViewMode viewMode() = 0;
  virtual void setViewMode(ViewMode view_mode) = 0;

  sigslot::signal<SelectionEvent> selection_signal;

  struct Params {
    Shared<DepthSampler> depth_sampler = DepthSampler::Create({});
    Eigen::Vector3d up_in_world =
        axisDirection<double, 3>(Conventions::global().up_direction_world);
    sophus2::Region3F64 camera_limits_in_world = sophus2::Region3F64::empty();
    ViewMode view_mode = ViewMode::best_guess;
    bool constrain_image_zoom_bounds = true;
  };
  static Shared<DrawLayerHandler> Create(const Params&);
};

}  // namespace pangolin
