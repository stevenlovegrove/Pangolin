#include "camera_utils.h"
#include "gl_utils.h"

#include <pangolin/drawable/drawn_plot_background.h>
#include <pangolin/gl/glsl_program.h>
#include <pangolin/gl/uniform.h>
#include <pangolin/render/gl_vao.h>
#include <pangolin/utils/shared.h>

namespace pangolin
{

struct GlDrawnPlotBackground : public DrawnPlotBackground {
  GlDrawnPlotBackground(const DrawnPlotBackground::Params& p)
  {
    auto bind_prog = prog->bind();
    u_color_background = p.color_background;
    u_tick_color_scale = p.tick_color_scale;
    u_num_divisions = p.num_divisions;
  }

  void draw(const ViewParams& params) override
  {
    auto bind_prog = prog->bind();
    auto bind_vao = vao.bind();
    auto disable_depth = ScopedGlDisable(GL_DEPTH_TEST);

    Eigen::Matrix4d graph_from_clip =
        (params.clip_from_image * params.image_from_camera *
         params.camera_from_drawable)
            .inverse();
    Eigen::Array2d min_in_graph =
        (graph_from_clip * Eigen::Vector4d(-1.0, -1.0, 0.0, 1.0)).head<2>();
    Eigen::Array2d max_in_graph =
        (graph_from_clip * Eigen::Vector4d(+1.0, +1.0, 0.0, 1.0)).head<2>();

    // range of the graph in graph units
    Eigen::Array2d range_in_graph = (max_in_graph - min_in_graph).abs();

    // how many graph units are there per pixel (graph units / pixels)
    Eigen::Array2d graph_units_per_pixel =
        range_in_graph.array() / params.viewport.range().cast<double>().array();

    // minimum distance between tics in pixels
    const double min_dist_in_pixels = 10.0;
    // minimum distance between tics in graph units
    Eigen::Array2d tics_min_dist_in_graph =
        min_dist_in_pixels * graph_units_per_pixel;

    // let s be the number of subdivisions ('grid lines') per octave.
    const double s = u_num_divisions.getValue();

    // log with base s of minimum distance between tics (in graph units)
    Eigen::Array2d log_s_min_dist = tics_min_dist_in_graph.log() / std::log(s);

    Eigen::Array2d log_s_of_octave_start = log_s_min_dist.ceil();

    u_graph_from_clip = graph_from_clip.cast<float>();
    u_graph_units_per_pixel = graph_units_per_pixel.cast<float>();
    u_log_s_min_dist = log_s_min_dist.cast<float>();
    u_log_s_of_octave_start = log_s_of_octave_start.cast<float>();
    u_octave_start = Eigen::Array2f(
        std::pow(s, log_s_of_octave_start.x()),
        std::pow(s, log_s_of_octave_start.y()));

    PANGO_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
  }

  Region3F64 boundsInParent() const override { return Region3F64::empty(); }

  private:
  const Shared<GlSlProgram> prog = GlSlProgram::Create(
      {.sources = {
           {.origin = "/components/pango_opengl/shaders/"
                      "main_plot_background.glsl"}}});
  GlVertexArrayObject vao = {};
  const GlUniform<Eigen::Matrix4f> u_graph_from_clip = {"graph_from_clip"};
  const GlUniform<Eigen::Vector4f> u_color_background = {"color_background"};
  const GlUniform<Eigen::Vector4f> u_tick_color_scale = {"tick_color_scale"};
  const GlUniform<Eigen::Vector2f> u_graph_units_per_pixel = {
      "graph_units_per_pixel"};
  const GlUniform<Eigen::Vector2f> u_log_s_min_dist = {"log_s_min_dist"};
  const GlUniform<Eigen::Vector2f> u_log_s_of_octave_start = {
      "log_s_of_octave_start"};
  const GlUniform<Eigen::Vector2f> u_octave_start = {"octave_start"};

  const GlUniform<float> u_num_divisions = {"num_divisions"};
};

Shared<DrawnPlotBackground> DrawnPlotBackground::Create(
    DrawnPlotBackground::Params p)
{
  return Shared<GlDrawnPlotBackground>::make(p);
}

}  // namespace pangolin
