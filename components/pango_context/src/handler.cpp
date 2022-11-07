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

// Rotate camera such that the direction under the cursor at
// ``down_state`` is once more under the cursor with ``state``
sophus::SE3d orientPointToPoint(
  const PointState& down_state,
  const PointState& state)
{
  const Eigen::Vector3d from = down_state.p_cam.normalized();
  const Eigen::Vector3d to = state.p_cam.normalized();
  const sophus::SO3d to_R_from = sophus::SO3d::rotThroughPoints(from, to);
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


// UNTESTED
sophus::SE3d fixUpDirIfNeeded(
  const sophus::SE3d& cam_T_world,
  const std::optional<Eigen::Vector3d>& up_in_world,
  DeviceXyz axis_convention
) {
  if(up_in_world) {
    const Eigen::Vector3d up_in_cam = cam_T_world * (*up_in_world);
    const Eigen::Vector3d convention_up = upDirectionInCamera<double>(axis_convention);
    if( (up_in_cam - convention_up).squaredNorm() > sophus::kEpsilon<double> ) {
      const sophus::SO3d up_R_conv = sophus::SO3d::rotThroughPoints(convention_up, up_in_cam);
      return sophus::SE3d(up_R_conv,Eigen::Vector3d::Zero()) * cam_T_world;
    }
  }
  return cam_T_world;
}

// NOT FULLY IMPLEMENTED
sophus::SE3d rotateAbout(
  const sophus::SE3d& cam_T_world,
  const Eigen::Vector3d& point_in_world,
  const std::optional<Eigen::Vector3d>& up_dir_in_world,
  const Eigen::Vector3d& rotation_vector,
  DeviceXyz axis_convention
) {
  // const Eigen::Vector3d axis_up_world = up_dir_in_world ? *up_dir_in_world : (cam_T_world.inverse() * upDirectionInCamera<double>(axis_convention));
  // const Eigen::Vector3d axis_fwd_world = (cam_T_world.inverse().translation() - point_in_world).normalized();
  // const Eigen::Vector3d axis_right_world = (axis1_world.cross(axis2_world)).normalized();

  // const sophus::SE3d fixed_T_world = fixUpDirIfNeeded(cam_T_world, up_dir_in_world, axis_convention);

  sophus::SE3d rot(sophus::SO3d::exp(rotation_vector), Eigen::Vector3d::Zero());
  sophus::SE3d point_T_cam(sophus::SO3d(), -(cam_T_world * point_in_world));
  return point_T_cam.inverse() * rot * point_T_cam * cam_T_world;
}

sophus::SE3d zoomTowards(
    const sophus::SE3d& cam_T_world,
    const Eigen::Vector3d& point_in_cam,
    const MinMax<double>& near_far,
    const double zoom_input
) {
  const Eigen::Vector3d vec = zoom_input * point_in_cam;
  return sophus::SE3d(sophus::SO3d(), vec) * cam_T_world;
}




class HandlerImpl : public Handler {
 public:
  enum class ViewMode {
    freeview
  };
  enum class CenterUpdate {
    with_click,        // Center of rotation is updated with every mouse-down event
    with_double_click, // Center of rotation is updated with every double-click to form
                       // a kind of 3D cursor
    fixed,             // e.g. programmatic object-oriented navigation
  };

  HandlerImpl(const Handler::Params& p)
      : depth_sampler_(p.depth_sampler)
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
        }
      }
    }

    const Eigen::Vector3d p_cam = camera.camUnproj(pix_img, zdepth_cam);
    const Eigen::Vector3d p_world = camera_from_world.inverse() * p_cam;
    const PointState state = {pix_img, p_cam, camera_from_world, zdepth_cam};

    std::visit(overload {
    [&](const Interactive::PointerEvent& arg) {
        if (arg.action == PointerAction::down) {
          down_state = state;
        }else if(arg.action == PointerAction::drag) {
          PANGO_ASSERT(down_state);
          // camera_from_world = orientPointToPoint(*down_state, state);
          camera_from_world = translatePointToPoint(*down_state, state);
        }
        // fmt::print("img: {}, zdepth_cam: {}\n", pix_img.transpose(), zdepth_cam );
    },
    [&](const Interactive::ScrollEvent& arg) {
        double zoom_input = std::clamp(-arg.pan[1]/200.0, -1.0, 1.0);
        camera_from_world = zoomTowards(camera_from_world, p_cam, near_far, zoom_input);
        // rot_center_in_world = p_world;
        //   camera_from_world = rotateAbout(
        //     camera_from_world, *rot_center_in_world,  Eigen::Vector3d(0,-1,0),
        //     Eigen::Vector3d(arg.pan[1], arg.pan[0], 0.0)/200.0, DeviceXyz::right_down_forward );
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
  std::optional<PointState> down_state;

  // Point to rotate about
  std::optional<Eigen::Vector3d> rot_center_in_world;

  // Mode of interpretting input
  ViewMode viewmode_ = ViewMode::freeview;

  // Optionally constrain view direction such that cameras left-to-right
  // axis is perpendicular to ``up_in_world``
  std::optional<Eigen::Vector3d> up_in_world;

  // Specify how the center of rotation is updated
  CenterUpdate rot_center_polixy;
};

std::unique_ptr<Handler> Handler::Create(Params const& p) {
  return std::make_unique<HandlerImpl>(p);
}

}  // namespace pangolin
