#include <pangolin/utils/fmt.h>
#include <pangolin/handler/handler.h>
#include <pangolin/utils/variant_overload.h>
#include <pangolin/utils/logging.h>

namespace pangolin {

class HandlerImpl : public Handler {
 public:
  enum class ViewMode {
    freeview
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
    const Context& context,
    const Interactive::Event& event
  ) override {

    const auto region = event.pointer_pos.region();
    const Eigen::Array2d p_window = event.pointer_pos.posInWindow();
    const Eigen::Array2d pix_img = event.pointer_pos.posInRegionNorm() * Eigen::Array2d(
        camera.imageSize().width, camera.imageSize().height
        ) - Eigen::Array2d(0.5,0.5);
    std::optional<DepthSampler::Sample> maybe_depth_sample;
    std::optional<Eigen::Vector3d> maybe_p_cam;
    std::optional<Eigen::Vector3d> maybe_p_world;
    std::optional<double> maybe_zdepth_cam;
    if(depth_sampler_) {
        maybe_depth_sample = depth_sampler_->sampleDepth(p_window.cast<int>(), 2, near_far, &context);
        if(maybe_depth_sample) {
            PANGO_CHECK(maybe_depth_sample->depth_kind == DepthSampler::DepthKind::zaxis);
            maybe_zdepth_cam = maybe_depth_sample->min_max.min();
            maybe_p_cam = camera.camUnproj(pix_img, *maybe_zdepth_cam);
            maybe_p_world = camera_from_world.inverse() * (*maybe_p_cam);
        }
    }

    std::visit(overload {
    [&](const Interactive::PointerEvent& arg) {
        if (arg.action == PointerAction::down) {
            fmt::print( "img: {}, zdepth_cam: {}, cam: {}, world: {}\n",
                pix_img.transpose(),
                maybe_zdepth_cam,
                maybe_p_cam,
                maybe_p_world
            );
        }
    },
    [&](const Interactive::ScrollEvent& arg) {
        Eigen::Vector3d x = {arg.pan[1], -arg.pan[0], 0.0};
        auto pan = sophus::SE3d(sophus::SO3d::exp(x / 500.0), Eigen::Vector3d::Zero());
        camera_from_world = pan * camera_from_world;
    },
    [](auto&&  arg) { PANGO_UNREACHABLE(); },
    }, event.detail);

    return true;
  }

  std::shared_ptr<DepthSampler> depth_sampler_;
  ViewMode viewmode_ = ViewMode::freeview;
};

std::unique_ptr<Handler> Handler::Create(Params const& p) {
  return std::make_unique<HandlerImpl>(p);
}

}  // namespace pangolin
