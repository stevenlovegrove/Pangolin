#include <pangolin/drawable/drawn_text.h>
#include <pangolin/gl/gl.h>
#include <pangolin/gl/gl_scoped_enable.h>
#include <pangolin/gl/gl_vao.h>
#include <pangolin/gl/glfont.h>
#include <pangolin/gl/glsl_program.h>
#include <pangolin/gl/uniform.h>
#include <pangolin/render/projection.h>
#include <pangolin/utils/shared.h>

#include <codecvt>
#include <locale>
#include <string>

namespace pangolin
{

template <typename T>
Eigen::Vector<T, 4> toHomogeneousVec4(const Eigen::Vector<T, 2>& x)
{
  return Eigen::Vector<T, 4>(
      x[0], x[1], static_cast<T>(0.0), static_cast<T>(1.0));
}

std::u32string toUtf32(const std::string& utf8)
{
  return std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}
      .from_bytes(utf8);
}

struct GlDrawnText : public DrawnText {
  struct TextBuffer {
    Eigen::Vector2d pos_in_drawable;
    double angle;
    pangolin::GlBuffer vbo_pos;
    pangolin::GlBuffer vbo_index;
    GlVertexArrayObject vao;
    double font_size_em = 1.0;
  };

  GlDrawnText(const DrawnText::Params& p)
  {
    // TODO: Allow customization
    font = build_builtin_font(
        p.font_height_pixels, p.font_height_pixels * 32,
        p.font_height_pixels * 32, false);
    font->InitialiseGlTexture();
    font_offsets.Load(font->MakeFontLookupImage());

    auto prog_bind = prog->bind();
    u_font_atlas = (int)0;
    u_font_offsets = 1;
    u_color = {0.0f, 0.0f, 0.0f};
  }

  void addText(
      const Eigen::Vector2d& pos_in_drawable, double angle,
      const std::u32string& utf32, const double font_size_em)
  {
    const std::u16string index16 = font->to_index_string(utf32);
    GlFont::codepoint_t last_char = 0;

    Eigen::Vector2d p = Eigen::Vector2d::Zero();
    std::vector<Eigen::Vector3f> host_vbo_pos;
    std::vector<uint16_t> host_vbo_index;

    for (size_t c = 0; c < index16.size(); ++c) {
      const GlFont::codepoint_t this_char = utf32[c];

      if (!index16[c]) {
        // TODO: use some symbol such as '?' maybe
        p.x() += font->default_advance_px;
        last_char = 0;
      } else {
        auto ch = font->chardata[this_char];

        if (last_char) {
          const auto key = GlFont::codepointpair_t(last_char, this_char);
          const auto kit = font->kern_table.find(key);
          const float kern = (kit != font->kern_table.end()) ? kit->second : 0;
          p.x() += kern;
        }

        host_vbo_pos.emplace_back(p.x(), p.y(), 0.0);
        host_vbo_index.emplace_back(index16[c]);
        p.x() += ch.StepX();
        last_char = this_char;
      }
    }

    auto t = Shared<TextBuffer>::make();
    t->font_size_em = font_size_em;
    t->pos_in_drawable = pos_in_drawable;
    t->angle = angle;
    t->vbo_pos = pangolin::GlBuffer(pangolin::GlArrayBuffer, host_vbo_pos);
    t->vbo_index = pangolin::GlBuffer(pangolin::GlArrayBuffer, host_vbo_index);
    t->vao.addVertexAttrib(0, t->vbo_pos);
    t->vao.addVertexAttrib(1, t->vbo_index);
    texts.push_back(std::move(t));
  }

  void addText(
      const Eigen::Vector2d& pos_parent, const std::string& utf8, double angle,
      const double font_size_em) override
  {
    addText(pos_parent, angle, toUtf32(utf8), font_size_em);
  }

  virtual void clearTexts() override { texts.clear(); }

  void draw(const ViewParams& p) override
  {
    ScopedGlDisable dis_depth(GL_DEPTH_TEST);
    auto bind_prog = prog->bind();

    u_font_bitmap_type = static_cast<int>(font->bitmap_type);
    u_max_sdf_dist_uv = {
        font->bitmap_max_sdf_dist_uv[0], font->bitmap_max_sdf_dist_uv[1]};

    const Eigen::Matrix4d clip_from_drawable =
        p.clip_from_image * p.image_from_camera * p.camera_from_drawable;
    Eigen::Matrix4d clip_from_pix_scale = Eigen::Matrix4d::Identity();

    for (auto& text : texts) {
      auto bind_vao = text->vao.bind();

      // positioned in scene units, but with pixel sized font
      const Eigen::Vector4d pos_in_clip4 =
          clip_from_drawable * toHomogeneousVec4(text->pos_in_drawable);
      const Eigen::Vector3d pos_in_clip =
          pos_in_clip4.head<3>() / pos_in_clip4.w();
      clip_from_pix_scale.diagonal().head<2>() =
          text->font_size_em * Eigen::Array2d(2.0, -2.0) /
          p.viewport.range().cast<double>().array();
      const Eigen::Matrix4d textclip_from_screenpix =
          sophus::SE3d(pos_in_clip).matrix() * clip_from_pix_scale *
          sophus::SE3d::fromRz(text->angle).matrix();
      u_clip_from_fontpix = textclip_from_screenpix.cast<float>();

      glActiveTexture(GL_TEXTURE0);
      font->mTex.Bind();
      glActiveTexture(GL_TEXTURE1);
      font_offsets.Bind();
      glDrawArrays(GL_POINTS, 0, text->vbo_index.num_elements);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  sophus::Region3F64 boundsInParent() const override
  {
    return sophus::Region3F64::empty();
  }

  Shared<GlSlProgram> prog = GlSlProgram::Create(
      {.sources = {
           {.origin = "/components/pango_opengl/shaders/main_text.glsl"}}});
  GlUniform<int> u_font_atlas = {prog, "u_font_atlas"};
  GlUniform<int> u_font_offsets = {prog, "u_font_offsets"};
  GlUniform<Eigen::Array3f> u_color = {prog, "u_color"};

  GlUniform<int> u_font_bitmap_type = {prog, "u_font_bitmap_type"};
  GlUniform<Eigen::Array2f> u_max_sdf_dist_uv = {prog, "u_max_sdf_dist_uv"};
  GlUniform<Eigen::Matrix4f> u_clip_from_fontpix = {
      prog, "u_clip_from_fontpix"};

  std::shared_ptr<GlFont> font;
  GlTexture font_offsets;

  std::vector<Shared<TextBuffer>> texts;
};

Shared<DrawnText> DrawnText::Create(DrawnText::Params p)
{
  return Shared<GlDrawnText>::make(p);
}

}  // namespace pangolin
