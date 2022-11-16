#include <pangolin/utils/fmt.h>
#include <pangolin/handler/handler.h>
#include <pangolin/utils/variant_overload.h>
#include <pangolin/utils/logging.h>
#include <sophus/geometry/ray.h>

namespace pangolin {

struct PointState {
  Eigen::Vector2d p_img;
  Eigen::Vector3d p_cam;
  sophus::Se3F64 cam_T_world;
  double z_depth_cam;
};

// Return the projection of ``point_in_foo`` onto the plane
// through the origin which is perpendicular to ``axis_in_foo``
Eigen::Vector3d componentPerpendicularToAxis(
  const Eigen::Vector3d& unit_axis_in_foo,
  const Eigen::Vector3d& point_in_foo
) {
  const double projection = unit_axis_in_foo.dot(point_in_foo);
  const Eigen::Vector3d point_on_axis = projection * unit_axis_in_foo;
  const Eigen::Vector3d perp = point_in_foo - point_on_axis;
  // std::cout << unit_axis_in_foo.dot(perp) << std::endl;
  return perp;
}

// UNTESTED
sophus::SO3d alignUpDir(
  const Eigen::Vector3d& unit_axis_in_cam,
  const Eigen::Vector3d& unit_up_in_cam,
  DeviceXyz axis_convention
) {
  // this is the vector we will be attempting to reorient to unit_up_in_cam
  const Eigen::Vector3d unit_camup_in_cam = upDirectionInCamera<double>(axis_convention);

  // we want axis, up and convention to all be in plane by rotating through axis.
  // Generate vectors a and b which represent rotation orthogonal to axis
  const Eigen::Vector3d a = componentPerpendicularToAxis(unit_axis_in_cam, unit_camup_in_cam).normalized();
  const Eigen::Vector3d b = componentPerpendicularToAxis(unit_axis_in_cam, unit_up_in_cam).normalized();

  // std::cout << "=======" << std::endl;
  // const Eigen::Vector3d axis_check = a.cross(b).normalized();
  // std::cout << unit_axis_in_cam.transpose() << std::endl;
  // std::cout << axis_check.transpose() << std::endl;

  // TODO: if either a or b are colinear with up, we probably have a problem.
  const sophus::SO3d b_R_a = sophus::rotThroughPoints(a, b);
  return b_R_a;
}

// Rotate camera such that the direction under the cursor at
// ``down_state`` is once more under the cursor with ``state``
sophus::SE3d orientPointToPoint(
  const PointState& down_state,
  const PointState& state,
  const std::optional<Eigen::Vector3d>& unit_up_in_world,
  const DeviceXyz axis_convention
) {
  const Eigen::Vector3d unit_x_from = down_state.p_cam.normalized();
  const Eigen::Vector3d unit_x_to = state.p_cam.normalized();

  // pointed frame has target aligned under mouse, but may introduce in-plane rotation
  const sophus::SO3d pointed_R_from = sophus::rotThroughPoints(unit_x_from, unit_x_to);

  sophus::SO3d to_R_from;

  // if(unit_up_in_world)
  if(true)
  {
    const sophus::SO3d to_R_pointed = alignUpDir(
      pointed_R_from * unit_x_from,
      pointed_R_from * down_state.cam_T_world.so3() * (*unit_up_in_world),
      axis_convention
    );
    to_R_from  = to_R_pointed * pointed_R_from;
  }else{
    to_R_from = pointed_R_from;
  }

  return sophus::SE3d(to_R_from, Eigen::Vector3d::Zero()) * down_state.cam_T_world;
}

// Translate camera such that the point in 3D under the cursor at
// ``down_state`` is once more under the cursor with ``state`` and
// having the same z_depth
sophus::SE3d translatePointToPoint(
  const PointState& down_state,
  const PointState& state)
{
  const Eigen::Vector3d from = down_state.p_cam;
  const Eigen::Vector3d to = from.z() * state.p_cam / state.p_cam.z();
  const Eigen::Vector3d to_in_from = to - from;
  // TODO: I would have expected to need from_in_to here, but this works.
  return sophus::SE3d(sophus::SO3d(), to_in_from) * down_state.cam_T_world;
}

sophus::SE3d rotateAroundAxis(
  const sophus::Ray3<double>& axis,
  double angle_rad
) {
  sophus::SE3d point_T_cam(sophus::SO3d(), -axis.origin());
  sophus::SE3d point_R_point(
    sophus::SO3d::exp(axis.direction().vector()*angle_rad),
    Eigen::Vector3d::Zero()
  );
  return point_T_cam.inverse() * point_R_point * point_T_cam;
}

// NOT FULLY IMPLEMENTED
sophus::SE3d rotateAbout(
  const sophus::SE3d& cam_T_world,
  const Eigen::Vector3d& point_in_world,
  const std::optional<Eigen::Vector3d>& up_dir_in_world,
  const Eigen::Vector3d& rotation_amount,
  DeviceXyz axis_convention
) {
  // const Eigen::Vector3d axis_up_world = up_dir_in_world ? *up_dir_in_world : (cam_T_world.inverse() * upDirectionInCamera<double>(axis_convention));
  // const Eigen::Vector3d axis_fwd_world = (cam_T_world.inverse().translation() - point_in_world).normalized();
  // const Eigen::Vector3d axis_right_world = (axis1_world.cross(axis2_world)).normalized();

  // const sophus::SE3d fixed_T_world = fixUpDirIfNeeded(cam_T_world, up_dir_in_world, axis_convention);

  Eigen::Vector3d up_dir_in_cam = cam_T_world.so3() * (*up_dir_in_world);
  Eigen::Vector3d point_in_cam = cam_T_world * point_in_world;
  sophus::SE3d rotx_T_world = rotateAroundAxis(
    sophus::Ray3<double>(point_in_cam, sophus::UnitVector3<double>::fromUnitVector(up_dir_in_cam)),
    rotation_amount.y()
  ) * cam_T_world;

  // up vector in camera frame is different after first rotation
  up_dir_in_cam = rotx_T_world.so3() * (*up_dir_in_world);
  point_in_cam = cam_T_world * point_in_world;
  Eigen::Vector3d right_dir_in_cam(1.0,0.0,0.0);
  return rotateAroundAxis(
    sophus::Ray3<double>(point_in_cam, sophus::UnitVector3<double>::fromUnitVector(right_dir_in_cam)),
    rotation_amount.x()
  ) * rotx_T_world;

  // sophus::SE3d rot(sophus::SO3d::exp(rotation_amount), Eigen::Vector3d::Zero());
  // sophus::SE3d point_T_cam(sophus::SO3d(), -(cam_T_world * point_in_world));
  // return point_T_cam.inverse() * rot * point_T_cam * cam_T_world;
}

void zoomTowards(
    sophus::CameraModel& camera,
    sophus::SE3d& cam_from_world,
    MinMax<Eigen::Vector3d>& camera_limits_in_world,
    const Eigen::Vector3d& point_in_cam,
    const MinMax<double>& near_far,
    const double zoom_input
) {
  using namespace sophus;
  if(camera.distortionType() == CameraDistortionType::orthographic) {
    OrthographicModel& ortho = std::get<OrthographicModel>(camera.modelVariant());
    static_assert(OrthographicModel::kNumParams == 4);
    double factor = 1.0 - zoom_input;
    if(ortho.params()[0] * factor < 1.0) factor = 1.0 / ortho.params()[0];
    auto scale_param = ortho.params().head<2>();
    scale_param *= factor;

    // adjust translation so that point in camera coordinates stays under cursor.
    const Eigen::Vector2d xy = point_in_cam.head<2>() * (1.0/factor - 1.0);
    cam_from_world = sophus::SE3d(sophus::SO3d(), Eigen::Vector3d(xy[0], xy[1], 0.0)) * cam_from_world;

    if(!camera_limits_in_world.empty()) {
      const Eigen::Vector2d orig(ortho.imageSize().width, ortho.imageSize().height);
      const Eigen::Vector2d diff = orig.array() - orig.array()/scale_param.array();
      const Eigen::Vector3d offset = -(ortho.camUnproj(Eigen::Vector2d(-0.5,-0.5), 0.0) + Eigen::Vector3d(0.5,0.5,0.0));

      camera_limits_in_world = MinMax<Eigen::Vector3d>(
        offset, {offset[0]+diff[0], offset[1]+diff[1], offset[2]}
      );
    }
  }else{
    const Eigen::Vector3d vec = zoom_input * point_in_cam;
    cam_from_world = sophus::SE3d(sophus::SO3d(), vec) * cam_from_world;
  }
}


class HandlerImpl : public Handler {
 public:
  enum class ViewMode {
    freeview
  };
  enum class CursorUpdate {
    with_click,        // Cursor of rotation is updated with every mouse-down event
    with_double_click, // Cursor of rotation is updated with every double-click to form
                       // a kind of 3D cursor
    fixed,             // e.g. programmatic object-oriented navigation
  };

  HandlerImpl(const Handler::Params& p)
      : depth_sampler_(p.depth_sampler), up_in_world_(p.up_in_world)
  {
  }

  bool handleEvent(
    DrawLayer& layer,
    sophus::Se3F64& camera_from_world,
    sophus::CameraModel& camera,
    MinMax<double>& near_far,
    MinMax<Eigen::Vector3d>& camera_limits_in_world,
    const Context& context,
    const Interactive::Event& event
  ) override {

    const auto region = event.pointer_pos.region();
    const Eigen::Array2d p_window = event.pointer_pos.posInWindow();
    const Eigen::Array2d pix_img = event.pointer_pos.posInRegionNorm() * Eigen::Array2d(
        camera.imageSize().width, camera.imageSize().height
        ) - Eigen::Array2d(0.5,0.5);

    double zdepth_cam = last_zcam_;

    if(depth_sampler_) {
      std::optional<DepthSampler::Sample> maybe_depth_sample =
        depth_sampler_->sampleDepth(p_window.cast<int>(), 2, near_far, &context);
      if(maybe_depth_sample) {
        PANGO_CHECK(maybe_depth_sample->depth_kind == DepthSampler::DepthKind::zaxis);
        if(maybe_depth_sample->min_max.min() < near_far.max() * 0.99) {
          zdepth_cam = maybe_depth_sample->min_max.min();
          last_zcam_ = zdepth_cam;
        }
      }
    }

    const Eigen::Vector3d p_cam = camera.camUnproj(pix_img, zdepth_cam);
    const Eigen::Vector3d p_world = camera_from_world.inverse() * p_cam;
    const PointState state = {pix_img, p_cam, camera_from_world, zdepth_cam};

    std::visit(overload {
    [&](const Interactive::PointerEvent& arg) {
        if (arg.action == PointerAction::down) {
          down_state_ = state;
        }else if(arg.action == PointerAction::drag) {
          if(!down_state_) {
            PANGO_WARN("Unexpected");
            return;
          }
          if(arg.button_active == PointerButton::primary) {
            camera_from_world = translatePointToPoint(*down_state_, state);
          // }else if(arg.button_active == PointerButton::secondary) {
          //   camera_from_world = orientPointToPoint(*down_state_, state, up_in_world_, axis_convention_);
          // }else if(arg.button_active == (PointerButton::primary | PointerButton::secondary) ) {
          //   // ... in plane rotation
          }
        }
    },
    [&](const Interactive::ScrollEvent& arg) {
        // double zoom_input = std::clamp(-arg.pan[1]/200.0, -1.0, 1.0);
        // zoomTowards(camera, camera_from_world, camera_limits_in_world, p_cam, near_far, zoom_input);

        // Don't allow the center of rotation change without a small delay
        // from scrolling - otherwise it is very easy to accidentally rotate
        // around a point that suddenly comes under the cursor.
        auto now = std::chrono::system_clock::now();
        if(now - last_cursor_update_ > cursor_update_wait_) {
          cursor_in_world_ = p_world;
        }
        last_cursor_update_ = now;

        camera_from_world = rotateAbout(
          camera_from_world, cursor_in_world_,  up_in_world_,
          Eigen::Vector3d(arg.pan[1], arg.pan[0], 0.0)/200.0, DeviceXyz::right_down_forward );

        double zoom_input = std::clamp(-arg.zoom/1.0, -1.0, 1.0);
        zoomTowards(camera, camera_from_world, camera_limits_in_world, camera_from_world * cursor_in_world_, near_far, zoom_input);
    },
    [](auto&&  arg) { PANGO_UNREACHABLE(); },
    }, event.detail);

    // Clamp translation to valid bounds in world coordinates
    if(!camera_limits_in_world.empty()) {
      auto world_from_cam = camera_from_world.inverse();
      world_from_cam.translation() = camera_limits_in_world.clamp(world_from_cam.translation());
      camera_from_world = world_from_cam.inverse();
    }

    return true;
  }

  double last_zcam_ = 1.0;

  // Delegate for querying depth under the cursor
  std::shared_ptr<DepthSampler> depth_sampler_;

  // State of viewpoint and pointer direction during button press
  std::optional<PointState> down_state_;

  // center for various zoom / rotate operations
  Eigen::Vector3d cursor_in_world_ = {0.0, 0.0, 0.0};

  // Specify how the center of rotation is updated
  // CursorUpdate cursor_center_polixy_;
  std::chrono::system_clock::time_point last_cursor_update_;
  std::chrono::system_clock::duration cursor_update_wait_ = std::chrono::milliseconds(200);

  // Mode of interpretting input
  ViewMode viewmode_ = ViewMode::freeview;

  // Optionally constrain view direction such that cameras left-to-right
  // axis is perpendicular to ``up_in_world``
  std::optional<Eigen::Vector3d> up_in_world_;

  DeviceXyz axis_convention_ = DeviceXyz::right_down_forward;


};

std::unique_ptr<Handler> Handler::Create(Params const& p) {
  return std::make_unique<HandlerImpl>(p);
}

}  // namespace pangolin
