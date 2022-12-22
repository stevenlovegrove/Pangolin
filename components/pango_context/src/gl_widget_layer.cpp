#include "gl_widget_layer.h"

#include "gl_utils.h"

#include <pangolin/context/context.h>
#include <pangolin/context/factory.h>
#include <pangolin/gl/glsl_program.h>
#include <pangolin/maths/eigen_scalar_methods.h>
#include <pangolin/maths/projection.h>
#include <pangolin/utils/file_utils.h>
#include <pangolin/utils/string.h>
#include <pangolin/var/var.h>
#include <pangolin/var/varextra.h>

#include <codecvt>
#include <locale>
#include <string>

namespace pangolin
{

GlWidgetLayer::GlWidgetLayer(const WidgetLayer::Params& p) :
    size_hint_(p.size_hint),
    name_(p.name),
    widget_height(p.scale * p.widget_height_pix),
    widget_padding(p.scale * p.widget_padding_pix),
    font_scale(p.scale * 0.5),
    scroll_offset(0.0),
    selected_widget(-1)
{
  font = build_builtin_font(32, 1024, 1024, false);
  font->InitialiseGlTexture();
  font_offsets.Load(font->MakeFontLookupImage());

  LoadShaders();

  // Receive Pangolin var events
  sigslot_lifetime = pangolin::VarState::I().RegisterForVarEvents(
      [this](const pangolin::VarState::Event& event) {
        process_var_event(event);
      },
      true);
}

std::string GlWidgetLayer::name() const { return name_; }

WidgetLayer::Size GlWidgetLayer::sizeHint() const { return size_hint_; }

void GlWidgetLayer::LoadShaders() { dirty = true; }

void GlWidgetLayer::UpdateWidgetParams()
{
  for (auto& wp : widgets) {
    if (wp.write_params) {
      wp.write_params(wp);
    }
  }
}

void GlWidgetLayer::UpdateWidgetVBO(float width)
{
  {
    auto bind_prog = widget_program.prog->bind();
    widget_program.width = width;
    widget_program.height = widget_height;
    widget_program.padding = widget_padding;
    widget_program.num_widgets = (int)widgets.size();
    widget_program.selected_index = hover_widget;
  }

  std::vector<Eigen::Vector4f> host_vbo;
  for (size_t i = 0; i < widgets.size(); ++i) {
    const auto& w = widgets[i];
    host_vbo.emplace_back(
        0.0, i, w.divisions + w.value_percent / 2.0, uint(w.widget_type));
  }
  widget_program.vbo = pangolin::GlBuffer(pangolin::GlArrayBuffer, host_vbo);
  widget_program.vao.addVertexAttrib(0, widget_program.vbo);
}

std::u32string GlWidgetLayer::toUtf32(const std::string& utf8)
{
  return std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}
      .from_bytes(utf8);
}

float GlWidgetLayer::TextWidthPix(const std::u32string& utf32)
{
  float w = 0;
  for (auto c : utf32) {
    w += font_scale * font->chardata[c].StepX();
  }
  return w;
}

void GlWidgetLayer::AddTextToHostBuffer(
    const std::u32string& utf32, Eigen::Array2d p,
    std::vector<Eigen::Vector3f>& host_vbo_pos,
    std::vector<uint16_t>& host_vbo_index)
{
  const std::u16string index16 = font->to_index_string(utf32);
  GlFont::codepoint_t last_char = 0;

  for (size_t c = 0; c < index16.size(); ++c) {
    const GlFont::codepoint_t this_char = utf32[c];

    if (!index16[c]) {
      // TODO: use some symbol such as '?' maybe
      p.x() += font_scale * font->default_advance_px;
      last_char = 0;
    } else {
      auto ch = font->chardata[this_char];

      if (last_char) {
        const auto key = GlFont::codepointpair_t(last_char, this_char);
        const auto kit = font->kern_table.find(key);
        const float kern = (kit != font->kern_table.end()) ? kit->second : 0;
        p.x() += font_scale * kern;
      }

      host_vbo_pos.emplace_back(p.x(), p.y(), 0.0);
      host_vbo_index.emplace_back(index16[c]);
      p.x() += font_scale * ch.StepX();
      last_char = this_char;
    }
  }
}

void GlWidgetLayer::UpdateCharsVBO(float widget_width)
{
  // set uniforms
  {
    auto bind_prog = text_program.prog->bind();
    text_program.font_bitmap_type = static_cast<int>(font->bitmap_type);
    text_program.scale = font_scale;
    text_program.max_sdf_dist_uv = {
        font->bitmap_max_sdf_dist_uv[0], font->bitmap_max_sdf_dist_uv[1]};
  }

  const float text_pad = 2.5 * widget_padding;

  std::vector<Eigen::Vector3f> host_vbo_pos;
  std::vector<uint16_t> host_vbo_index;
  for (size_t i = 0; i < widgets.size(); ++i) {
    const auto& w = widgets[i];

    // y-position is roughly center with fudge factor since text is balanced
    // low.
    const float y_pos =
        (i + 0.5) * widget_height + 0.3 * font_scale * font->font_height_px;

    AddTextToHostBuffer(
        toUtf32(w.text), {text_pad, y_pos - 0.175 * font->font_height_px},
        host_vbo_pos, host_vbo_index);

    //            if(w.widget_type == WidgetType::slider)
    if (!w.value.empty()) {
      const auto utf32 = toUtf32(w.value);
      const float width = TextWidthPix(utf32);
      AddTextToHostBuffer(
          utf32,
          {widget_width - text_pad - width,
           y_pos + 0.175 * font->font_height_px},
          host_vbo_pos, host_vbo_index);
    }
  }

  text_program.vbo_pos =
      pangolin::GlBuffer(pangolin::GlArrayBuffer, host_vbo_pos);
  text_program.vbo_index =
      pangolin::GlBuffer(pangolin::GlArrayBuffer, host_vbo_index);
  text_program.vao.addVertexAttrib(0, text_program.vbo_pos);
  text_program.vao.addVertexAttrib(1, text_program.vbo_index);
  text_program.vbo_index.Unbind();
}

void GlWidgetLayer::renderIntoRegion(
    const Context& context, const RenderParams& p)
{
  ScopedGlEnable en_scissor(GL_SCISSOR_TEST);
  ScopedGlDisable dis_depth(GL_DEPTH_TEST);
  glEnable(GL_BLEND);  // Assume this is okay to keep
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  context.setViewport(p.region);
  glClearColor(0.85f, 0.85f, 0.85f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  const auto size = p.region.range();

  const auto region = p.region.translated(-p.region.min())
                          .cast<Eigen::Vector2d>()
                          .translated({-0.5, -(0.5 + scroll_offset)});

  T_cm = projectionClipFromOrtho(
             region, {-1.0, 1.0}, ImageXy::right_down,
             ImageIndexing::pixel_centered)
             .cast<float>();

  // TODO: try to do this less. It's expensive
  {
    UpdateWidgetParams();
    UpdateWidgetVBO(size[0]);
    UpdateCharsVBO(size[0]);
  }

  {
    auto bind_prog = widget_program.prog->bind();
    auto bind_vao = widget_program.vao.bind();
    widget_program.T_cm = T_cm;
    glDrawArrays(GL_POINTS, 0, widget_program.vbo.num_elements);
  }

  {
    auto bind_prog = text_program.prog->bind();
    auto bind_vao = text_program.vao.bind();
    text_program.T_cm = T_cm;

    glActiveTexture(GL_TEXTURE0);
    font->mTex.Bind();
    glActiveTexture(GL_TEXTURE1);
    font_offsets.Bind();
    glDrawArrays(GL_POINTS, 0, text_program.vbo_index.num_elements);
    font->mTex.Unbind();
  }

  glActiveTexture(GL_TEXTURE0);
}

bool GlWidgetLayer::handleEvent(const Context&, const Event& event)
{
  const auto region = event.pointer_pos.region;
  const Eigen::Array2d p_window = event.pointer_pos.pos_window;

  std::visit(
      overload{
          [&](const Interactive::PointerEvent& arg) {
            bool pressed = arg.action == PointerAction::down;
            auto w = WidgetXY(p_window, region);

            switch (arg.action) {
              case PointerAction::down:
              case PointerAction::click_up: {
                if (selected_widget >= 0) {
                  auto& sw = widgets[selected_widget];
                  SetValue(p_window, region, pressed, false);
                  if (!pressed) {
                    selected_widget = -1;
                  }
                } else {
                  selected_widget = w.first;
                  SetValue(p_window, region, pressed, false);
                }
                break;
              }
              case PointerAction::drag: {
                SetValue(p_window, region, pressed, true);
                break;
              }
              default:
                break;
            }
          },
          [&](const Interactive::ScrollEvent& arg) {
            const float delta = arg.pan.y();
            const float offset_max = (widgets.size() - 1.0f) * widget_height;
            scroll_offset =
                std::clamp(scroll_offset + delta, -offset_max, 0.0f);
          },
          [&](const Interactive::KeyboardEvent& arg) {
            auto w = WidgetXY(p_window, region);
            if (0 <= w.first && w.first < (int)widgets.size()) {
              WidgetParams& wp = widgets[w.first];
              const double delta_mag = wp.divisions ? 1.0 / wp.divisions : 0.01;
              const double delta = (arg.key == 228)
                                       ? -delta_mag
                                       : (arg.key == 230 ? +delta_mag : 0.0);
              if (delta == 0.0 || !arg.pressed) return;
              if (wp.widget_type == WidgetType::slider) {
                wp.value_percent =
                    std::clamp(wp.value_percent + delta, 0.0, 1.0);
                if (wp.read_params) wp.read_params(wp);
                dirty = true;
              }
            }

            // PANGO_INFO("key: {}", (int)arg.key);
          },
          [](auto&& arg) { PANGO_UNREACHABLE(); },
      },
      event.detail);

  return true;
}

void GlWidgetLayer::SetValue(
    const Eigen::Array2d& p, const MinMax<Eigen::Array2i>& region, bool pressed,
    bool dragging)
{
  auto w = WidgetXY(p, region);
  if (w.first == selected_widget && 0 <= w.first &&
      w.first < (int)widgets.size()) {
    WidgetParams& wp = widgets[w.first];
    if (wp.widget_type == WidgetType::checkbox) {
      if (pressed && !dragging) wp.value_percent = 1.0 - wp.value_percent;
    } else if (wp.widget_type == WidgetType::button) {
      wp.value_percent = pressed ? 0.0 : 1.0;
    } else {
      if (pressed || dragging) {
        const float val = std::clamp(
            (w.second.x() - widget_padding) /
                (region.range().x() - 2 * widget_padding),
            0.0f, 1.0f);

        if (wp.divisions == 0) {
          // continuous version
          wp.value_percent = val;
        } else {
          // springy discrete version
          const float d = (std::round(val * wp.divisions) / wp.divisions);
          float diff = val - d;
          wp.value_percent = d + diff * sping_coeff;
        }
      } else if (!pressed) {
        // de-springify
        if (wp.divisions) {
          wp.value_percent =
              (std::round(wp.value_percent * wp.divisions) / wp.divisions);
        }
      }
    }

    if (wp.read_params) {
      wp.read_params(wp);
    }

    dirty = true;
  }
}

std::pair<int, Eigen::Vector2f> GlWidgetLayer::WidgetXY(
    const Eigen::Array2d& p, const MinMax<Eigen::Array2i>& region)
{
  const Eigen::Vector2f p_view(
      p.x() - region.min().x(), p.y() - region.min().y() - scroll_offset);
  const int i = std::floor(p_view[1] / widget_height);
  const Eigen::Vector2f p_widget(
      p_view[0], std::fmod(p_view[1], widget_height));
  return {i, p_widget};
}

void GlWidgetLayer::process_var_event(const pangolin::VarState::Event& event)
{
  using namespace pangolin;

  // ignore ones we're not subscibed to.
  if (!startsWith(event.var->Meta().full_name, name())) return;

  if (event.action == VarState::Event::Action::Added) {
    auto var = event.var;

    if (!strcmp(var->TypeId(), typeid(bool).name())) {
      widgets.push_back(WidgetParams{
          event.var->Meta().friendly,
          "",
          1.0f,
          0,
          var->Meta().flags & META_FLAG_TOGGLE ? WidgetType::checkbox
                                               : WidgetType::button,
          [var](const WidgetParams& p) {  // read_params
            Var<bool> v(var);
            v = p.value_percent > 0.5;
            v.Meta().gui_changed = true;
          },
          [var](WidgetParams& p) {  // write params
            Var<bool> v(var);
            p.value_percent = v ? 1.0 : 0.0;
          },
      });
    } else if (
        !strcmp(var->TypeId(), typeid(double).name()) ||
        !strcmp(var->TypeId(), typeid(float).name()) ||
        !strcmp(var->TypeId(), typeid(size_t).name()) ||
        !strcmp(var->TypeId(), typeid(int8_t).name()) ||
        !strcmp(var->TypeId(), typeid(uint8_t).name()) ||
        !strcmp(var->TypeId(), typeid(int16_t).name()) ||
        !strcmp(var->TypeId(), typeid(uint16_t).name()) ||
        !strcmp(var->TypeId(), typeid(int32_t).name()) ||
        !strcmp(var->TypeId(), typeid(uint32_t).name()) ||
        !strcmp(var->TypeId(), typeid(int64_t).name()) ||
        !strcmp(var->TypeId(), typeid(uint64_t).name())) {
      const bool is_integral = strcmp(var->TypeId(), typeid(double).name()) &&
                               strcmp(var->TypeId(), typeid(float).name());

      auto& r = var->Meta().range;
      const double range = r[1] - r[0];
      const double steps =
          is_integral ? range : (range / var->Meta().increment);
      widgets.push_back(WidgetParams{
          event.var->Meta().friendly,
          "",
          1.0f,
          static_cast<int>(steps),
          WidgetType::slider,
          [var, is_integral](const WidgetParams& p) {  // read_params
            Var<double> v(var);
            auto& r = var->Meta().range;
            const double range = r[1] - r[0];
            const double val =
                std::clamp(r[0] + range * p.value_percent, r[0], r[1]);
            v = is_integral ? std::round(val) : val;
            v.Meta().gui_changed = true;
          },
          [var, is_integral](WidgetParams& p) {  // write params
            // TODO: this is breaking the 'springyness' for integral sliders
            // because reading from the int value is flooring the value_percent.
            Var<double> v(var);
            const double val = v.Get();
            auto& r = var->Meta().range;
            const double range = r[1] - r[0];
            const double from_percent_val =
                std::clamp(r[0] + range * p.value_percent, r[0], r[1]);
            if (!is_integral ||
                std::round(from_percent_val) != std::round(val)) {
              p.value_percent = std::clamp((val - r[0]) / range, 0.0, 1.0);
            }
            p.value = var->str->Get();
          },
      });
      widgets.back().write_params(widgets.back());
    } else if (!strcmp(
                   var->TypeId(), typeid(std::function<void(void)>).name())) {
      widgets.push_back(WidgetParams{
          event.var->Meta().friendly,
          "",
          1.0f,
          0,
          WidgetType::button,
          [var](const WidgetParams& p) {  // read_params
            Var<std::function<void(void)>> v(var);
            if (p.value_percent > 0.5) v.Get()();
          },
          {}});
    } else if (var->str) {
      widgets.push_back(WidgetParams{
          event.var->Meta().friendly,
          "",
          1.0f,
          0,
          WidgetType::textbox,
          [var](const WidgetParams& p) {  // read_params
          },
          [var](WidgetParams& p) {  // write params
            Var<std::string> v(var);
            p.value = v.Get();
          },
      });
    }

    dirty = true;
  } else if (event.action == pangolin::VarState::Event::Action::Removed) {
    PANGO_UNIMPLEMENTED();
  }
}

PANGO_CREATE(WidgetLayer) { return Shared<GlWidgetLayer>::make(p); }

}  // namespace pangolin
