#include "camera_utils.h"
#include "gl_utils.h"

#include <pangolin/context/factory.h>
#include <pangolin/gl/glsl_program.h>
#include <pangolin/gl/uniform.h>
#include <pangolin/gui/drawn_checker.h>
#include <pangolin/render/gl_vao.h>

namespace pangolin
{

struct GlDrawnChecker : public DrawnChecker {
  GlDrawnChecker(const DrawnChecker::Params& p)
  {
    auto bind_prog = prog->bind();
    u_color1 = p.check_color_1;
    u_color2 = p.check_color_2;
    u_checksize = p.check_size_pixels;
  }

  void draw(const ViewParams& params) override
  {
    auto bind_prog = prog->bind();
    auto bind_vao = vao.bind();
    auto disable_depth = ScopedGlDisable(GL_DEPTH_TEST);
    u_viewport_size = params.viewport.range().cast<float>();
    PANGO_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
  }

  Interval<Eigen::Vector3d> boundsInParent() const override
  {
    return Interval<Eigen::Vector3d>::closed();
  }

  private:
  const Shared<GlSlProgram> prog = GlSlProgram::Create(
      {.sources = {
           {.origin = "/components/pango_opengl/shaders/main_checker.glsl"}}});
  GlVertexArrayObject vao = {};
  const GlUniform<Eigen::Vector2f> u_viewport_size = {"viewport_size"};
  const GlUniform<Eigen::Vector4f> u_color1 = {"color1"};
  const GlUniform<Eigen::Vector4f> u_color2 = {"color2"};
  const GlUniform<int> u_checksize = {"checksize"};
};

PANGO_CREATE(DrawnChecker) { return Shared<GlDrawnChecker>::make(p); }

}  // namespace pangolin
