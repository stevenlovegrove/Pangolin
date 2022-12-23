#pragma once

#include <pangolin/gl/gl.h>
#include <pangolin/gl/glfont.h>
#include <pangolin/gl/glsl_program.h>
#include <pangolin/gl/uniform.h>
#include <pangolin/gui/widget_layer.h>
#include <pangolin/render/gl_vao.h>
#include <pangolin/var/var.h>
#include <pangolin/var/varextra.h>

#include <codecvt>
#include <locale>
#include <string>

namespace pangolin
{

struct GlWidgetLayer : WidgetLayer {
  GlWidgetLayer(const WidgetLayer::Params& p);

  std::string name() const override;

  Size sizeHint() const override;

  void renderIntoRegion(const Context&, const RenderParams&) override;

  bool handleEvent(const Context&, const Event&) override;

  ///////////

  static constexpr float sping_coeff = 0.2;

  enum class WidgetType {
    label = 0,
    textbox,
    button,
    checkbox,
    slider,
    seperator
  };

  struct WidgetParams {
    std::string text;
    std::string value;
    float value_percent;
    int divisions;
    WidgetType widget_type;
    std::function<void(const WidgetParams&)> read_params;
    std::function<void(WidgetParams&)> write_params;
  };

  void LoadShaders();

  void UpdateWidgetParams();

  void UpdateWidgetVBO(float width);

  std::u32string toUtf32(const std::string& utf8);

  float TextWidthPix(const std::u32string& utf32);

  void AddTextToHostBuffer(
      const std::u32string& utf32, Eigen::Array2d p,
      std::vector<Eigen::Vector3f>& host_vbo_pos,
      std::vector<uint16_t>& host_vbo_index);

  void UpdateCharsVBO(float width);

  void SetValue(
      const Eigen::Array2d& p, const MinMax<Eigen::Array2i>& region,
      bool pressed, bool dragging);

  std::pair<int, Eigen::Vector2f> WidgetXY(
      const Eigen::Array2d& p, const MinMax<Eigen::Array2i>& region);

  void process_var_event(const pangolin::VarState::Event& event);

  struct WidgetProgram {
    WidgetProgram()
    {
      // Set constant uniforms that wont change
      auto bind_prog = prog->bind();

      slider_outline_border = 2.0f;
      boss_border = 1.0f;
      boss_radius_factor = 1.0f;

      color_panel = {0.85f, 0.85f, 0.85f};
      color_boss_base = {0.8f, 0.8f, 0.8f};
      color_boss_diff = {0.2f, 0.15f, 0.20f};
      color_slider = {0.9f, 0.7f, 0.7f};
      color_slider_outline = {0.8f, 0.6f, 0.6f};
    }

    const Shared<GlSlProgram> prog = GlSlProgram::Create(
        {.sources = {
             {.origin =
                  "/components/pango_opengl/shaders/main_widgets.glsl"}}});
    GlUniform<float> width = {"u_width"};
    GlUniform<float> height = {"u_height"};
    GlUniform<float> padding = {"u_padding"};
    GlUniform<int> num_widgets = {"u_num_widgets"};
    GlUniform<int> selected_index = {"u_selected_index"};

    GlUniform<float> slider_outline_border = {"slider_outline_border"};
    GlUniform<float> boss_border = {"boss_border"};
    GlUniform<float> boss_radius_factor = {"boss_radius_factor"};

    GlUniform<Eigen::Array3f> color_panel = {"color_panel"};
    GlUniform<Eigen::Array3f> color_boss_base = {"color_boss_base"};
    GlUniform<Eigen::Array3f> color_boss_diff = {"color_boss_diff"};
    GlUniform<Eigen::Array3f> color_slider = {"color_slider"};
    GlUniform<Eigen::Array3f> color_slider_outline = {"color_slider_outline"};

    GlUniform<Eigen::Matrix4f> clip_from_pix = {"u_T_cm"};

    pangolin::GlBuffer vbo;
    GlVertexArrayObject vao = {};
  };

  struct TextProgram {
    TextProgram()
    {
      auto prog_bind = prog->bind();
      font_atlas = (int)0;
      font_offsets = 1;
      color = {0.0f, 0.0f, 0.0f};
    }

    const Shared<GlSlProgram> prog = GlSlProgram::Create(
        {.sources = {
             {.origin = "/components/pango_opengl/shaders/main_text.glsl"}}});
    GlUniform<int> font_atlas = {"u_font_atlas"};
    GlUniform<int> font_offsets = {"u_font_offsets"};
    GlUniform<Eigen::Array3f> color = {"u_color"};

    GlUniform<int> font_bitmap_type = {"u_font_bitmap_type"};
    GlUniform<Eigen::Array2f> max_sdf_dist_uv = {"u_max_sdf_dist_uv"};

    GlUniform<Eigen::Matrix4f> clip_from_pix = {"u_clip_from_fontpix"};

    pangolin::GlBuffer vbo_pos;
    pangolin::GlBuffer vbo_index;
    GlVertexArrayObject vao;
  };

  Size size_hint_;
  std::string name_;
  bool dirty;
  std::shared_ptr<GlFont> font;
  GlTexture font_offsets;
  Eigen::Matrix<float, 4, 4> clip_from_pix;

  WidgetProgram widget_program;
  TextProgram text_program;
  bool reload_shader = false;
  float widget_height;
  float widget_padding;
  int font_height_pix;
  float scroll_offset;
  int selected_widget;
  int hover_widget;
  std::vector<WidgetParams> widgets;
  sigslot::connection sigslot_lifetime;
};

}  // namespace pangolin
