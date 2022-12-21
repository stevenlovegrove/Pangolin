#include "camera_utils.h"

#include <pangolin/context/factory.h>
#include <pangolin/gl/glsl_program.h>
#include <pangolin/gl/uniform.h>
#include <pangolin/gui/drawn_solids.h>
#include <pangolin/render/gl_vao.h>

namespace pangolin
{

struct GlDrawnSolids : public DrawnSolids {
  void draw(ViewParams const& params) override
  {
    auto bind_prog = prog->bind();
    auto bind_vao = vao.bind();

    u_cam_from_clip = (params.clip_from_image * params.image_from_camera)
                          .inverse()
                          .cast<float>();
    u_world_from_cam =
        params.camera_from_world.inverse().cast<float>().matrix();
    u_znear_zfar =
        Eigen::Vector2f(params.near_far.min(), params.near_far.max());

    std::optional<ScopedBind<DeviceTexture>> bind_unprojmap;
    if (params.unproject_map && !params.unproject_map->empty()) {
      PANGO_GL(glActiveTexture(GL_TEXTURE0));
      bind_unprojmap = params.unproject_map->bind();
      u_use_unproject_map = true;
    }

    PANGO_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
  }

  MinMax<Eigen::Vector3d> boundsInParent() const override
  {
    return MinMax<Eigen::Vector3d>();
  }

  private:
  Shared<GlSlProgram> const prog = GlSlProgram::Create(
      {.sources = {
           {.origin = "/components/pango_opengl/shaders/main_plane.glsl"}}});
  GlVertexArrayObject vao = {};
  GlUniform<Eigen::Matrix4f> const u_cam_from_clip = {"camera_from_clip"};
  GlUniform<Eigen::Matrix4f> const u_world_from_cam = {"world_from_cam"};
  GlUniform<Eigen::Vector2f> const u_znear_zfar = {"znear_zfar"};
  GlUniform<bool> const u_use_unproject_map = {"use_unproject_map"};
};

PANGO_CREATE(DrawnSolids)
{
  auto r = Shared<GlDrawnSolids>::make();
  return r;
}

}  // namespace pangolin
