#pragma once

#include <pangolin/render/framebuffer_reader.h>

namespace pangolin {

class GlFramebufferReader : public FramebufferReader {
 public:
  // acceptable values for mode are:
  // GL_FRONT_LEFT, GL_FRONT_RIGHT, GL_BACK_LEFT, GL_BACK_RIGHT, GL_FRONT,
  // GL_BACK, GL_LEFT, GL_RIGHT and the constants GL_COLOR_ATTACHMENT(i)
  //
  // framebuffer_id is 0 for the screen backed render buffer, or the framebuffer
  // id as you would use for a glBindFramebuffer(...) call.;
  //
  // Generally, to read depth from an off-screen buffer, use
  // GL_COLOR_ATTACHMENT0
  GlFramebufferReader(
      MinMax<float> near_far,
      int gl_framebuffer_id = 0,
      unsigned int gl_mode = 0x0404 /* GL_FRONT */);

  farm_ng::Expected<MinMax<double>> getDepth(
      Eigen::Vector2i const& pix,
      int patch_rad,
      DepthKind = DepthKind::zaxis) const override;

 private:
  sophus::MutImage<float> readPatch(
      Eigen::Vector2i const& pix_center,
      int patch_rad,
      unsigned int gl_format,
      unsigned int gl_type) const;

  MinMax<float> near_far_;
  int gl_framebuffer_id_;
  unsigned int gl_mode_;
};

}  // namespace pangolin
