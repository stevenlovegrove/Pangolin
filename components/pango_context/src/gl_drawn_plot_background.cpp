#include "camera_utils.h"
#include "gl_utils.h"

#include <pangolin/context/factory.h>
#include <pangolin/gl/glsl_program.h>
#include <pangolin/gl/uniform.h>
#include <pangolin/gui/drawn_plot_background.h>
#include <pangolin/render/gl_vao.h>

namespace pangolin
{

struct GlDrawnPlotBackground : public DrawnPlotBackground {
  GlDrawnPlotBackground(const DrawnPlotBackground::Params& p)
  {
    auto bind_prog = prog->bind();
    u_color_background = p.color_background;
    u_tick_color_scale = p.tick_color_scale;
    u_tick_to_tick = p.tick_to_tick;
  }

  void draw(const ViewParams& params) override
  {
    auto bind_prog = prog->bind();
    auto bind_vao = vao.bind();
    auto disable_depth = ScopedGlDisable(GL_DEPTH_TEST);
    double t2t = u_tick_to_tick.getValue();

    Eigen::Matrix4d graph_from_clip =
        (params.clip_from_image * params.image_from_camera *
         params.camera_from_world)
            .inverse();
    Eigen::Array2d graph_min =
        (graph_from_clip * Eigen::Vector4d(-1.0, -1.0, 0.0, 1.0)).head<2>();
    Eigen::Array2d graph_max =
        (graph_from_clip * Eigen::Vector4d(+1.0, +1.0, 0.0, 1.0)).head<2>();
    Eigen::Array2d graph_range = (graph_max - graph_min).abs();
    Eigen::Array2d graph_per_pix =
        graph_range / params.viewport.range().cast<double>();
    Eigen::Array2d pixmin_in_graph = 10.0 * graph_per_pix;
    Eigen::Array2d log10_view = pixmin_in_graph.log() / std::log(t2t);
    Eigen::Array2d log10_start = log10_view.ceil();

    u_graph_from_clip = graph_from_clip.cast<float>();
    u_graph_per_pix = graph_per_pix.cast<float>();
    u_log10_view = log10_view.cast<float>();
    u_log10_start = log10_start.cast<float>();
    u_unit_graph_start = Eigen::Array2f(
        std::pow(t2t, log10_start.x()), std::pow(t2t, log10_start.y()));

    PANGO_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
  }

  MinMax<Eigen::Vector3d> boundsInParent() const override
  {
    return MinMax<Eigen::Vector3d>::closed();
  }

  private:
  const Shared<GlSlProgram> prog = GlSlProgram::Create(
      {.sources = {
           {.origin = "/components/pango_opengl/shaders/"
                      "main_plot_background.glsl"}}});
  GlVertexArrayObject vao = {};
  const GlUniform<Eigen::Matrix4f> u_graph_from_clip = {"graph_from_clip"};
  const GlUniform<Eigen::Vector4f> u_color_background = {"color_background"};
  const GlUniform<Eigen::Vector4f> u_tick_color_scale = {"tick_color_scale"};
  const GlUniform<Eigen::Vector2f> u_graph_per_pix = {"graph_per_pix"};
  const GlUniform<Eigen::Vector2f> u_log10_view = {"log10_view"};
  const GlUniform<Eigen::Vector2f> u_log10_start = {"log10_start"};
  const GlUniform<Eigen::Vector2f> u_unit_graph_start = {"unit_graph_start"};

  const GlUniform<float> u_tick_to_tick = {"tick_to_tick"};
};

PANGO_CREATE(DrawnPlotBackground)
{
  return Shared<GlDrawnPlotBackground>::make(p);
}

}  // namespace pangolin
