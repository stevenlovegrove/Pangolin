#include <pangolin/gl/glplatform.h>
#include "gl_framebuffer_reader.h"

namespace pangolin {

GlFramebufferReader::GlFramebufferReader(
    MinMax<float> near_far, GLint gl_framebuffer_id, GLenum gl_mode)
    : near_far_(near_far),
      gl_framebuffer_id_(gl_framebuffer_id),
      gl_mode_(gl_mode) {
  bool const mode_compatible_with_screen_buffer =
      GL_FRONT_LEFT <= gl_mode_ && gl_mode_ <= GL_BACK;

  if (gl_framebuffer_id_ == 0 && !mode_compatible_with_screen_buffer) {
    FARM_LOG_WARNING("Bad configuration.");
  }
  if (gl_framebuffer_id_ > 0 && mode_compatible_with_screen_buffer) {
    FARM_LOG_WARNING("Bad configuration");
  }
}

// Scales a depth value `real_depth`, in world units, to the range [0, 1]
// Derived from https://stackoverflow.com/questions/6652253
float glDepthFromReal(float real_depth, float znear, float zfar) {
  float const A = -2.0 * znear * zfar;
  float const B = zfar - znear;
  float const C = zfar + znear;

  // Scale to normalized device coordinates, [-1, 1]
  float const z_n = (A / real_depth + C) / B;

  // Scale to the range [0, 1]
  float z_b = (z_n + 1.0) / 2.0;

  return z_b;
}

float realDepthFromGl(float gl_depth, float znear, float zfar) {
  // Scale to normalized device coordinates, [-1, 1]
  float const A = -2.0 * znear * zfar;
  float const B = zfar - znear;
  float const C = zfar + znear;

  // Scale to normalized device coordinates, [-1, 1] from [0,1]
  float const z_n = 2.0 * gl_depth - 1.0;

  // Scale to linear depth range
  float const real_depth = A / (B * z_n - C);

  return real_depth;
}

Eigen::Vector2i windowCoordFromViewportTopLeft(Eigen::Vector2i& p) {
  // TODO: since we want to be GL agnostic, we should accept
  //       pixel locations in a common frame.
  //       For GL, we endup converting back and forth, but mouse
  //       interaction is an uncommon event.
  return p;
}

// Extract a patch of pixels from the selected framebuffer and attachment
// Convert read normalized depth to depth in scene units using the specified
// near and far clipping planes
//
// Possible GL State Changed:
//   * Bound framebuffer
//   * Current Draw Buffer
sophus::MutImage<float> GlFramebufferReader::readPatch(
    Eigen::Vector2i const& pix_center,
    int patch_rad,
    unsigned int format,
    unsigned int type) const {
  int const patch_dim = 2 * patch_rad + 1;

  sophus::MutImage<float> patch(sophus::ImageSize(patch_dim, patch_dim));

  glBindFramebuffer(GL_FRAMEBUFFER_EXT, gl_framebuffer_id_);
  glDrawBuffer(gl_mode_);

  glReadPixels(
      pix_center.x() - patch_rad,
      pix_center.y() - patch_rad,
      patch_dim,
      patch_dim,
      format,
      type,
      patch.ptrMut());

  patch.mutate([this](float gl_z) {
    return realDepthFromGl(gl_z, near_far_.min(), near_far_.max());
  });

  return patch;
}

farm_ng::Expected<MinMax<double>> GlFramebufferReader::getDepth(
    Eigen::Vector2i const& pix_center,
    int patch_rad,
    DepthKind depth_kind) const {
  auto const patch =
      readPatch(pix_center, patch_rad, GL_DEPTH_COMPONENT, GL_FLOAT);

  return finiteMinMax(patch).cast<double>();
}

}  // namespace pangolin
