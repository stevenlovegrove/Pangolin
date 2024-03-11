#include <pangolin/drawable/drawn_checker.h>
#include <pangolin/gl/gl_scoped_enable.h>
#include <pangolin/gl/gl_vao.h>
#include <pangolin/gl/glsl_program.h>
#include <pangolin/gl/uniform.h>
#include <pangolin/render/camera_utils.h>
#include <pangolin/utils/shared.h>

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

  sophus2::Region3F64 boundsInParent() const override
  {
    return sophus2::Region3F64::empty();
  }

  private:
  Shared<GlSlProgram> prog = GlSlProgram::Create(
      {.sources = {
           {.origin = "/components/pango_opengl/shaders/main_checker.glsl"}}});
  GlVertexArrayObject vao = {};
  GlUniform<Eigen::Vector2f> u_viewport_size = {prog, "viewport_size"};
  GlUniform<Eigen::Vector4f> u_color1 = {prog, "color1"};
  GlUniform<Eigen::Vector4f> u_color2 = {prog, "color2"};
  GlUniform<int> u_checksize = {prog, "checksize"};
};

Shared<DrawnChecker> DrawnChecker::Create(DrawnChecker::Params p)
{
  return Shared<GlDrawnChecker>::make(p);
}

}  // namespace pangolin
