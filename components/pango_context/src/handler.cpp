#include <fmt/format.h>
#include <pangolin/handler/handler.h>
#include <pangolin/utils/variant_overload.h>
#include <pangolin/utils/logging.h>


namespace pangolin {

class HandlerImpl : public Handler {
 public:
  HandlerImpl(const Handler::Params& p)
      : depth_sampler_(p.depth_sampler)
  {
  }

  bool handleEvent(
    DrawLayer& layer,
    sophus::Se3F64& world_from_camera,
    sophus::CameraModel& camera,
    MinMax<double>& near_far,
    const Context& context,
    const Interactive::Event& event
  ) override {

    const auto region = event.pointer_pos.region();
    const Eigen::Array2d p_window = event.pointer_pos.posInWindow();
    const Eigen::Array2d pix_img = event.pointer_pos.posInRegionNorm() * Eigen::Array2d(
        camera.imageSize().width,
        camera.imageSize().height
        );

    std::visit(overload {
    [&](const Interactive::PointerEvent& arg) {
        PANGO_INFO("PointerEvent");

        if (arg.action == PointerAction::down) {
          std::optional<DepthSampler::Sample> maybe_depth_sample;
          if(depth_sampler_) {
            maybe_depth_sample = depth_sampler_->sampleDepth(p_window.cast<int>(), 5, near_far, &context);
          }

          if (maybe_depth_sample) {
              PANGO_CHECK(maybe_depth_sample->depth_kind == DepthSampler::DepthKind::zaxis);
              double const zdepth_cam = maybe_depth_sample->min_max.min();
              const Eigen::Vector3d p_cam = camera.camUnproj(pix_img, zdepth_cam);
              const Eigen::Vector3d p_world = world_from_camera * p_cam;
              fmt::print(
                  "img: {}, zdepth_cam: {}, cam: {}, world: {}\n",
                  pix_img.transpose(),
                  zdepth_cam,
                  p_cam.transpose(),
                  p_world.transpose());
          }
        }
    },
    [](const Interactive::ScrollEvent& arg) {
      PANGO_INFO("ScrollEvent");
    },
    [](auto&&  arg) { PANGO_UNREACHABLE(); },
    }, event.detail);

    return true;
  }

  std::shared_ptr<DepthSampler> depth_sampler_;
};

std::unique_ptr<Handler> Handler::Create(Params const& p) {
  return std::make_unique<HandlerImpl>(p);
}

}  // namespace pangolin
