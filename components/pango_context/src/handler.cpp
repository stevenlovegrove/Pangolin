#include <fmt/format.h>
#include <pangolin/handler/handler.h>

namespace pangolin {

class HandlerImpl : public Handler {
 public:
  HandlerImpl(
      std::shared_ptr<FramebufferReader> const& framebuffer_reader,
      std::shared_ptr<sophus::Se3F64> const& world_from_camera,
      std::shared_ptr<sophus::CameraModel> const& camera)
      : framebuffer_reader_(framebuffer_reader),
        world_from_camera_(world_from_camera),
        camera_(camera) {}

  sophus::Se3F64 const& worldFromCamera() const override {
    return *world_from_camera_;
  }

  FramebufferReader const& framebufferReader() const override {
    return *framebuffer_reader_;
  }

  sophus::CameraModel const& camera() const override { return *camera_; }

  // Eigen::Array2d imagePixelFromWindow(View& v, Eigen::Array2d const& p_window) {
  //   // Define viewport coordinates (continuous, edge to edge)
  //   const Eigen::Array2d viewport_continuous(
  //       p_window.x() - v.v.l + 0.5, p_window.y() - v.v.b + 0.5);

  //   const Eigen::Array2d norm_yup =
  //       viewport_continuous / Eigen::Array2d(v.v.w, v.v.h);
  //   const Eigen::Array2d norm_ydown(norm_yup.x(), 1.0 - norm_yup.y());
  //   return norm_ydown *
  //              Eigen::Array2d(
  //                  camera().imageSize().width, camera().imageSize().height) -
  //          Eigen::Array2d(0.5, 0.5);
  // }

  // void Keyboard(View&, unsigned char key, int x, int y, bool pressed) override {
  // }

  // void Mouse(
  //     View& v, MouseButton button, int x, int y, bool pressed, int button_state)
  //     override {
  //   if (pressed) {
  //     const Eigen::Vector2i p_window(x, y);

  //     auto maybe_min_max_depth =
  //         framebuffer_reader->getDepth(p_window, 5);

  //     if (maybe_min_max_depth) {
  //       double const zdepth_cam = maybe_min_max_depth->min();

  //       const Eigen::Vector2d pix_img =
  //           imagePixelFromWindow(v, p_window.cast<double>());
  //       const Eigen::Vector3d p_cam = camera_->camUnproj(pix_img, zdepth_cam);
  //       const Eigen::Vector3d p_world = *world_from_camera_ * p_cam;
  //       fmt::print(
  //           "img: {}, cam: {}, world: {}\n",
  //           pix_img.transpose(),
  //           p_cam.transpose(),
  //           p_world.transpose());
  //     }

  //     // framebuffer_query_object_->
  //   }
  // }

  // void MouseMotion(View&, int x, int y, int button_state) override {}

  // void PassiveMouseMotion(View&, int x, int y, int button_state) override {}

  // void Special(
  //     View&,
  //     InputSpecial inType,
  //     float x,
  //     float y,
  //     float p1,
  //     float p2,
  //     float p3,
  //     float p4,
  //     int button_state) override {}

  std::shared_ptr<FramebufferReader> framebuffer_reader_;
  std::shared_ptr<sophus::Se3F64> world_from_camera_;
  std::shared_ptr<sophus::CameraModel> camera_;
};

std::unique_ptr<Handler> Handler::Create(Params const& p) {
  return std::make_unique<HandlerImpl>(
      p.framebuffer_reader, p.world_from_camera, p.camera);
}

}  // namespace pangolin
