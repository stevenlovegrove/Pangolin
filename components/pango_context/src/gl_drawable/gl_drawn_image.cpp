#include <pangolin/drawable/drawn_image.h>
#include <pangolin/gl/gl_scoped_enable.h>
#include <pangolin/gl/gl_vao.h>
#include <pangolin/gl/glsl_program.h>
#include <pangolin/gl/uniform.h>
#include <pangolin/render/camera_utils.h>
#include <pangolin/utils/shared.h>

namespace pangolin
{

struct GlDrawnImage : public DrawnImage {
  // This program renders an image on the z=0 plane in a discrete pixel
  // convention such that x=0,y=0 is the center of the top-left pixel.
  // As such the vertex extremes for the texture map from (-0.5,-0.5) to
  // (w-0.5,h-0.5).
  // TODO: Add a flag to take convention as configuration.

  void draw(const ViewParams& params) override
  {
    // ensure we're synced
    image->sync();

    PANGO_GL(glActiveTexture(GL_TEXTURE0));
    auto bind_im = image->bind();
    auto bind_prog = prog->bind();
    auto bind_vao = vao.bind();
    auto disable_depth = ScopedGlDisable(GL_DEPTH_TEST);

    u_image_size = toEigen(image->imageSize()).cast<float>();
    u_intrinsics =
        (params.clip_from_image * params.image_from_camera).cast<float>();
    u_cam_from_drawable = params.camera_from_drawable.cast<float>();

    if (color_transform) {
      u_color_transform = color_transform->cast<float>();
    } else if (
        image->pixelFormat().num_channels == 1 &&
        u_colormap_index.getValue() == Palette::none) {
      u_color_transform =
          (Eigen::Matrix4f() << 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1)
              .finished();
    } else {
      u_color_transform = Eigen::Matrix4f::Identity();
    }

    u_colormap_index = colormap;

    PANGO_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
  }

  sophus::Region3F64 boundsInParent() const override
  {
    return sophus::Region3F64::fromMinMax(
        Eigen::Vector3d(-0.5, -0.5, 1.0),
        Eigen::Vector3d(
            image->imageSize().width - 0.5, image->imageSize().height - 0.5,
            1.0));
  }

  private:
  Shared<GlSlProgram> prog = GlSlProgram::Create(
      {.sources = {
           {.origin = "/components/pango_opengl/shaders/main_image.glsl"}}});
  GlVertexArrayObject vao = {};
  GlUniform<int> texture_unit = {prog, "image"};
  GlUniform<Eigen::Matrix4f> u_intrinsics = {prog, "proj"};
  GlUniform<Eigen::Matrix4f> u_cam_from_drawable = {prog, "cam_from_world"};
  GlUniform<Eigen::Vector2f> u_image_size = {prog, "image_size"};
  GlUniform<Eigen::Matrix4f> u_color_transform = {prog, "color_transform"};
  GlUniform<Palette> u_colormap_index = {prog, "colormap_index"};
};

Shared<DrawnImage> DrawnImage::Create(DrawnImage::Params p)
{
  auto r = Shared<GlDrawnImage>::make();
  r->colormap = p.colormap;
  r->interpolation = p.interpolation;
  r->color_transform = p.color_transform;
  if (!p.image.isEmpty()) {
    r->image->update(p.image);
  }
  return r;
}

}  // namespace pangolin
