#include <pangolin/gui/draw_layer.h>
#include <pangolin/gui/draw_layer_handler.h>
#include <pangolin/utils/fmt.h>
#include <pangolin/utils/logging.h>
#include <pangolin/utils/variant_overload.h>
#include <sophus/geometry/ray.h>

#include "camera_utils.h"

namespace pangolin
{

// Pointer / touch state
struct PointerState {
  Eigen::Vector2d p_clip;
  Eigen::Vector2d p_img;
  Eigen::Vector3d p_cam;
  sophus::Se3<double> cam_T_world;
  sophus::Sim2<double> clip_view_transform;
  double z_depth_cam;
};

struct MouseUpdateArgs {
  // Potentially updated by function
  DrawLayerRenderState& render_state;
  MinMax<Eigen::Vector3d>& camera_limits_in_world;
  Eigen::Vector3d point_in_world;
  // Information
  PointerState pointer_now;
  PointerState pointer_pressed;
  Eigen::Array2d clip_aspect_scale;
  ImageXy image_convention;
  DeviceXyz axis_convention;
  std::optional<Eigen::Vector3d> unit_up_in_world;
};

///////////////////////////////////////////////////////////////////////////
// Util maths

// Return the projection of ``point_in_foo`` onto the plane
// through the origin which is perpendicular to ``axis_in_foo``
Eigen::Vector3d componentPerpendicularToAxis(
    const Eigen::Vector3d& unit_axis_in_foo,
    const Eigen::Vector3d& point_in_foo)
{
  const double projection = unit_axis_in_foo.dot(point_in_foo);
  const Eigen::Vector3d point_on_axis = projection * unit_axis_in_foo;
  const Eigen::Vector3d perp = point_in_foo - point_on_axis;
  return perp;
}

// Return a transform which represents a rotation by angle_rad around axis.
sophus::SE3d rotateAroundAxis(
    const sophus::Ray3<double>& axis, double angle_rad)
{
  sophus::SE3d point_T_cam(sophus::SO3d(), -axis.origin());
  sophus::SE3d point_R_point(
      sophus::SO3d::exp(axis.direction().vector() * angle_rad),
      Eigen::Vector3d::Zero());
  return point_T_cam.inverse() * point_R_point * point_T_cam;
}

// NOT WELL TESTED (probably broken)
sophus::SO3d alignUpDir(
    const Eigen::Vector3d& unit_axis_in_cam,
    const Eigen::Vector3d& unit_up_in_cam, DeviceXyz axis_convention)
{
  // this is the vector we will be attempting to reorient to unit_up_in_cam
  const Eigen::Vector3d unit_camup_in_cam =
      upDirectionInCamera<double>(axis_convention);

  // we want axis, up and convention to all be in plane by rotating through
  // axis. Generate vectors a and b which represent rotation orthogonal to axis
  const Eigen::Vector3d a =
      componentPerpendicularToAxis(unit_axis_in_cam, unit_camup_in_cam)
          .normalized();
  const Eigen::Vector3d b =
      componentPerpendicularToAxis(unit_axis_in_cam, unit_up_in_cam)
          .normalized();

  // TODO: if either a or b are colinear with up, we probably have a problem.
  const sophus::SO3d b_R_a = sophus::rotThroughPoints(a, b);
  return b_R_a;
}

///////////////////////////////////////////////////////////////////////////
// Camera motion (modifying cam_T_world)

// NOT WELL TESTED
// Rotate camera such that the direction under the cursor at
// ``down_state`` is once more under the cursor with ``state``
void cameraOrientPointToPoint(MouseUpdateArgs& info)
{
  const Eigen::Vector3d unit_x_from = info.pointer_pressed.p_cam.normalized();
  const Eigen::Vector3d unit_x_to = info.pointer_now.p_cam.normalized();

  // pointed frame has target aligned under mouse, but may introduce in-plane
  // rotation
  const sophus::SO3d pointed_R_from =
      sophus::rotThroughPoints(unit_x_from, unit_x_to);

  sophus::SO3d to_R_from;

  if (info.unit_up_in_world) {
    const sophus::SO3d to_R_pointed = alignUpDir(
        pointed_R_from * unit_x_from,
        pointed_R_from * info.pointer_pressed.cam_T_world.so3() *
            (*info.unit_up_in_world),
        info.axis_convention);
    to_R_from = to_R_pointed * pointed_R_from;
  } else {
    to_R_from = pointed_R_from;
  }

  info.render_state.camera_from_world =
      sophus::SE3d(to_R_from, Eigen::Vector3d::Zero()) *
      info.pointer_pressed.cam_T_world;
}

// Translate camera such that the point in 3D under the cursor at
// ``down_state`` is once more under the cursor with ``state`` and
// having the same z_depth
void cameraTranslatePointToPoint(MouseUpdateArgs& info)
{
  const Eigen::Vector3d from = info.pointer_pressed.p_cam;
  const Eigen::Vector3d to =
      from.z() * info.pointer_now.p_cam / info.pointer_now.p_cam.z();
  const Eigen::Vector3d to_in_from = to - from;
  info.render_state.camera_from_world =
      sophus::SE3d(sophus::SO3d(), to_in_from) *
      info.pointer_pressed.cam_T_world;
}

// Rotate about point_in_world maintaining up_dir_in_world relative to camera,
// such that point_in_world, camera_center and up_dir_in_world lie in a common
// plane.
void cameraRotateAbout(
    MouseUpdateArgs& info, const Eigen::Vector3d& rotation_amount)
{
  PANGO_ENSURE(info.unit_up_in_world);

  Eigen::Vector3d up_dir_in_cam =
      info.render_state.camera_from_world.so3() * (*info.unit_up_in_world);
  Eigen::Vector3d point_in_cam =
      info.render_state.camera_from_world * info.point_in_world;
  sophus::SE3d rotx_T_world =
      rotateAroundAxis(
          sophus::Ray3<double>(
              point_in_cam,
              sophus::UnitVector3<double>::fromUnitVector(up_dir_in_cam)),
          rotation_amount.y()) *
      info.render_state.camera_from_world;

  // up vector in camera frame is different after first rotation
  up_dir_in_cam = rotx_T_world.so3() * (*info.unit_up_in_world);
  point_in_cam = info.render_state.camera_from_world * info.point_in_world;
  Eigen::Vector3d right_dir_in_cam(1.0, 0.0, 0.0);
  info.render_state.camera_from_world =
      rotateAroundAxis(
          sophus::Ray3<double>(
              point_in_cam,
              sophus::UnitVector3<double>::fromUnitVector(right_dir_in_cam)),
          rotation_amount.x()) *
      rotx_T_world;
}

void cameraMoveTowardsPoint(MouseUpdateArgs& info, const double zoom_input)
{
  const Eigen::Vector3d vec =
      zoom_input * (info.render_state.camera_from_world * info.point_in_world);
  info.render_state.camera_from_world =
      sophus::SE3d(sophus::SO3d(), vec) * info.render_state.camera_from_world;
}

///////////////////////////////////////////////////////////////////////////
// Image plane motion (modifying CameraModel)

// Translate camera such that the point in 3D under the cursor at
// ``down_state`` is once more under the cursor with ``state`` and
// having the same z_depth
void imagePointToPoint(MouseUpdateArgs& info)
{
  sophus::Sim2<double> new_old;
  new_old.translation() = info.pointer_now.p_clip - info.pointer_pressed.p_clip;
  info.render_state.clip_view_transform =
      new_old * info.pointer_pressed.clip_view_transform;
}

void clampClipViewTransform(sophus::Sim2<double>& clip_view_transform)
{
  const double scale = clip_view_transform.scale();
  // Scale 1.0 means we're at min size (occupies entire viewport), so min/max
  // translation would be +-0. Scale 2.0 means we're zoomed in my 2. We can move
  // half the viewport in each direction which corresponds to min/max of 1 in
  // clip space. Scale 4.0 means min/max would be 3
  clip_view_transform.translation().x() = std::clamp(
      clip_view_transform.translation().x(), -(scale - 1.0), +(scale - 1.0));
  clip_view_transform.translation().y() = std::clamp(
      clip_view_transform.translation().y(), -(scale - 1.0), +(scale - 1.0));
}

void imageZoom(MouseUpdateArgs& info, const double zoom_input)
{
  using namespace sophus;
  double factor = 1.0 - zoom_input;

  sophus::Sim2<double> center_on_pointer;
  center_on_pointer.translation() = -info.pointer_now.p_clip;

  sophus::Sim2<double> new_old;
  new_old.setScale(factor);

  info.render_state.clip_view_transform = center_on_pointer.inverse() *
                                          new_old * center_on_pointer *
                                          info.render_state.clip_view_transform;

  info.render_state.clip_view_transform.setScale(
      std::max(1.0, info.render_state.clip_view_transform.scale()));
}

void imagePan(MouseUpdateArgs& info, const Eigen::Array2d& pan)
{
  sophus::Sim2<double> new_old;
  new_old.translation() =
      info.clip_aspect_scale * pan * Eigen::Array2d(1.0, -1.0);
  info.render_state.clip_view_transform =
      new_old * info.render_state.clip_view_transform;
}

class HandlerImpl : public DrawLayerHandler
{
  public:
  HandlerImpl(const DrawLayerHandler::Params& p) :
      depth_sampler_(p.depth_sampler),
      up_in_world_(p.up_in_world),
      view_mode_(p.view_mode)
  {
  }

  ViewMode viewMode() override { return view_mode_; }

  void setViewMode(ViewMode view_mode) override { view_mode_ = view_mode; }

  bool handleEvent(
      const Context& context, const Interactive::Event& event,
      const Eigen::Array2d& p_clip, const Eigen::Array2d& p_img,
      Eigen::Array2d clip_aspect_scale, DrawLayer& layer,
      DrawLayerRenderState& render_state) override
  {
    using namespace sophus;

    const CameraModel camera = render_state.camera;
    const MinMax<double> near_far = render_state.near_far;
    Se3<double>& camera_from_world = render_state.camera_from_world;
    Sim2<double>& clip_view_transform = render_state.clip_view_transform;

    auto camera_limits_in_world = MinMax<Eigen::Vector3d>::open();

    double zdepth_cam = last_zcam_;

    if (depth_sampler_) {
      DepthSampler::SampleLocation location = {
          .pos_camera_pixel = p_img,
          .pos_window = event.pointer_pos.pos_window};
      std::optional<DepthSampler::Sample> maybe_depth_sample =
          depth_sampler_->sampleDepth(location, 5, near_far, &context);
      if (maybe_depth_sample) {
        PANGO_CHECK(
            maybe_depth_sample->depth_kind == DepthSampler::DepthKind::zaxis);
        if (maybe_depth_sample->min_max.min() < near_far.max() * 0.99) {
          zdepth_cam = maybe_depth_sample->min_max.min();
          last_zcam_ = zdepth_cam;
        }
      }
    }

    const Eigen::Vector3d p_cam = camera.camUnproj(p_img, zdepth_cam);
    const Eigen::Vector3d p_world = camera_from_world.inverse() * p_cam;
    const PointerState state = {
        p_clip,    p_img, p_cam, camera_from_world, clip_view_transform,
        zdepth_cam};

    MouseUpdateArgs info = {render_state,        camera_limits_in_world,
                            cursor_in_world_,    state,
                            *down_state_,        clip_aspect_scale,
                            ImageXy::right_down, DeviceXyz::right_down_forward,
                            up_in_world_};

    std::visit(
        overload{
            [&](const Interactive::PointerEvent& arg) {
              if (arg.action == PointerAction::down) {
                down_state_ = state;
                cursor_in_world_ = p_world;
              } else if (arg.action == PointerAction::drag) {
                if (!down_state_) {
                  PANGO_WARN("Unexpected");
                  return;
                }
                if (arg.button_active == PointerButton::primary &&
                    !(event.modifier_active & ModifierKey::shift)) {
                  if (view_mode_ == ViewMode::image_plane) {
                    imagePointToPoint(info);
                  } else {
                    cameraTranslatePointToPoint(info);
                  }
                } else if (arg.button_active == PointerButton::secondary) {
                  if (view_mode_ == ViewMode::image_plane) {
                    // imagePointToPoint(info);
                  } else {
                    PANGO_ENSURE(down_state_);
                    const Eigen::Vector2d p1 = down_state_->p_clip.array() *
                                               Eigen::Array2d(1.0, -1.0) /
                                               clip_aspect_scale;
                    const Eigen::Vector2d p2 = state.p_clip.array() *
                                               Eigen::Array2d(1.0, -1.0) /
                                               clip_aspect_scale;
                    const Eigen::Vector2d diff = p2 - p1;
                    const Eigen::Vector3d rotation_amount(
                        diff.y(), diff.x(), 0.0);
                    cameraRotateAbout(info, rotation_amount);
                    down_state_ = state;
                  }
                }
              }

              // When holding shift, selection is made
              if (event.modifier_active & ModifierKey::shift) {
                auto selection = MinMax<Eigen::Array2d>(p_img);
                if (down_state_) {
                  selection.extend(down_state_->p_img);
                }

                selection_signal(SelectionEvent{
                    .trigger_event = event,
                    .in_pixel_selection = selection,
                    .in_progress = arg.action == PointerAction::down ||
                                   arg.action == PointerAction::drag});
              }
            },
            [&](const Interactive::ScrollEvent& arg) {
              const double zoom_input = std::clamp(-arg.zoom / 1.0, -1.0, 1.0);

              if (view_mode_ == ViewMode::image_plane) {
                cursor_in_world_ = p_world;
                const Eigen::Vector2d pan =
                    Eigen::Vector2d(arg.pan[0], arg.pan[1]).array() / 200.0;
                imagePan(info, pan);
                imageZoom(info, zoom_input);
              } else {
                // Don't allow the center of rotation change without a small
                // delay from scrolling - otherwise it is very easy to
                // accidentally rotate around a point that suddenly comes under
                // the cursor.
                auto now = std::chrono::system_clock::now();
                auto diff = now - last_cursor_update_;
                if (diff > cursor_update_wait_) {
                  cursor_in_world_ = p_world;
                }
                last_cursor_update_ = now;

                cameraRotateAbout(
                    info, Eigen::Vector3d(arg.pan[1], arg.pan[0], 0.0) / 200.0);
                cameraMoveTowardsPoint(info, zoom_input);
              }
            },
            [&](const Interactive::KeyboardEvent& arg) {
              // do nothing for now
            },
            [](auto&& arg) { PANGO_UNREACHABLE(); },
        },
        event.detail);

    clampClipViewTransform(info.render_state.clip_view_transform);

    return true;
  }

  double last_zcam_ = 1.0;

  // Delegate for querying depth under the cursor
  std::shared_ptr<DepthSampler> depth_sampler_;

  // State of viewpoint and pointer direction during button press
  std::optional<PointerState> down_state_;

  // center for various zoom / rotate operations
  Eigen::Vector3d cursor_in_world_ = {0.0, 0.0, 0.0};

  // Specify how the center of rotation is updated
  std::chrono::system_clock::time_point last_cursor_update_;
  std::chrono::system_clock::duration cursor_update_wait_ =
      std::chrono::milliseconds(200);

  // Optionally constrain view direction such that cameras left-to-right
  // axis is perpendicular to ``up_in_world``
  std::optional<Eigen::Vector3d> up_in_world_;

  DeviceXyz axis_convention_ = DeviceXyz::right_down_forward;

  // Mode of interpretting input
  ViewMode view_mode_;
};

Shared<DrawLayerHandler> DrawLayerHandler::Create(Params const& p)
{
  return Shared<HandlerImpl>::make(p);
}

}  // namespace pangolin
