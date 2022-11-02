#include <pangolin/context/factory.h>
#include <pangolin/utils/file_utils.h>
#include <pangolin/gl/glsl.h>
#include <pangolin/var/var.h>
#include <pangolin/var/varextra.h>
#include <locale>
#include <string>
#include <codecvt>
#include "glutils.h"

#include "gl_widget_layer.h"

extern const unsigned char AnonymousPro_ttf[];


namespace pangolin
{
namespace
{
constexpr GLint GLSL_LOCATION_CHAR_INDEX = DEFAULT_LOCATION_POSITION + 1;
}


std::shared_ptr<GlFont> build_builtin_font(float pixel_height, int tex_w, int tex_h, bool use_alpha_font)
{
    return std::make_shared<GlFont>(AnonymousPro_ttf, pixel_height, tex_w, tex_h, use_alpha_font);
}

GlWidgetLayer::GlWidgetLayer(const WidgetLayer::Params& p)
    : size_hint_(p.size_hint),
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
        [this](const pangolin::VarState::Event& event){
            process_var_event(event);
        }, true
        );
}

WidgetLayer::Size GlWidgetLayer::sizeHint() const {
    return size_hint_;
}

void GlWidgetLayer::LoadShaders()
{
    const std::string shader_dir = "/components/pango_opengl/shaders/";
    const std::string shader_widget = shader_dir + "main_widgets.glsl";
    const std::string shader_text = shader_dir + "main_text.glsl";

    CheckGlDieOnError();

    prog_widget.AddShaderFromFile(pangolin::GlSlAnnotatedShader, shader_widget, {}, {shader_dir});
    glBindAttribLocation(prog_widget.ProgramId(), DEFAULT_LOCATION_POSITION, DEFAULT_NAME_POSITION);
    prog_widget.Link();
    CheckGlDieOnError();

    prog_text.AddShaderFromFile(pangolin::GlSlAnnotatedShader, shader_text, {}, {shader_dir});
    glBindAttribLocation(prog_text.ProgramId(), DEFAULT_LOCATION_POSITION, DEFAULT_NAME_POSITION);
    glBindAttribLocation(prog_text.ProgramId(), GLSL_LOCATION_CHAR_INDEX, "a_char_index");
    prog_text.Link();
    CheckGlDieOnError();

    dirty = true;
}

void GlWidgetLayer::UpdateWidgetParams()
{
    for(auto& wp : widgets) {
        if(wp.write_params) {
            wp.write_params(wp);
        }
    }
}

void GlWidgetLayer::UpdateWidgetVBO(float width)
{
    prog_widget.Bind();
    prog_widget.SetUniform("u_width",  width );
    prog_widget.SetUniform("u_height", widget_height );
    prog_widget.SetUniform("u_padding", widget_padding );
    prog_widget.SetUniform("u_num_widgets", (int)widgets.size() );
    prog_widget.SetUniform("u_selected_index",  hover_widget);


    prog_widget.SetUniform("slider_outline_border", 2.0f);
    prog_widget.SetUniform("boss_border", 1.0f);
    prog_widget.SetUniform("boss_radius_factor", 1.0f);

    prog_widget.SetUniform("color_panel",          0.85f, 0.85f, 0.85f);
    prog_widget.SetUniform("color_boss_base",      0.8f, 0.8f, 0.8f);
    prog_widget.SetUniform("color_boss_diff",      0.2f, 0.15f, 0.20f);
    prog_widget.SetUniform("color_slider",         0.9f, 0.7f, 0.7f);
    prog_widget.SetUniform("color_slider_outline", 0.8f, 0.6f, 0.6f);
    prog_widget.Unbind();

    std::vector<Eigen::Vector4f> host_vbo;
    for(size_t i=0; i < widgets.size(); ++i) {
        const auto& w = widgets[i];
        host_vbo.emplace_back(0.0, i, w.divisions + w.value_percent/2.0, uint(w.widget_type) );
    }

    vbo_widgets = pangolin::GlBuffer( pangolin::GlArrayBuffer, host_vbo );
    vao_widgets.AddVertexAttrib(pangolin::DEFAULT_LOCATION_POSITION, vbo_widgets);
    vao_widgets.Unbind();
}

std::u32string GlWidgetLayer::toUtf32(const std::string& utf8)
{
    return std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.from_bytes(utf8);
}

float GlWidgetLayer::TextWidthPix(const std::u32string& utf32){
    float w = 0;
    for(auto c: utf32) {
        w += font_scale * font->chardata[c].StepX();
    }
    return w;
}

void GlWidgetLayer::AddTextToHostBuffer(
    const std::u32string& utf32, float x, float y,
    std::vector<Eigen::Vector3f>& host_vbo_pos,
    std::vector<uint16_t>& host_vbo_index)
{
    const std::u16string index16 = font->to_index_string(utf32);
    GlFont::codepoint_t last_char = 0;

    for(size_t c=0; c < index16.size(); ++c) {
        const GlFont::codepoint_t this_char = utf32[c];

        if(!index16[c]) {
            // TODO: use some symbol such as '?' maybe
            x += font_scale * font->default_advance_px;
            last_char = 0;
        }else{
            auto ch = font->chardata[this_char];

            if(last_char) {
                const auto key = GlFont::codepointpair_t(last_char,this_char);
                const auto kit = font->kern_table.find(key);
                const float kern = (kit != font->kern_table.end()) ? kit->second : 0;
                x += font_scale * kern;
            }

            host_vbo_pos.emplace_back(x, y, 0.0 );
            host_vbo_index.emplace_back(index16[c]);
            x += font_scale * ch.StepX();
            last_char = this_char;
        }
    }
}

void GlWidgetLayer::UpdateCharsVBO(float widget_width)
{
    prog_text.Bind();
    prog_text.SetUniform("u_font_atlas", 0);
    prog_text.SetUniform("u_font_offsets", 1);
    prog_text.SetUniform("u_font_bitmap_type", static_cast<int>(font->bitmap_type) );
    prog_text.SetUniform("u_scale", font_scale);
    prog_text.SetUniform("u_max_sdf_dist_uv", font->bitmap_max_sdf_dist_uv[0], font->bitmap_max_sdf_dist_uv[1] );
    prog_text.SetUniform("u_color", 0.0f, 0.0f, 0.0f);
    prog_text.Unbind();
    const float text_pad = 2.5*widget_padding;

    std::vector<Eigen::Vector3f> host_vbo_pos;
    std::vector<uint16_t> host_vbo_index;
    for(size_t i=0; i < widgets.size(); ++i) {
        const auto& w = widgets[i];

        // y-position is roughly center with fudge factor since text is balanced low.
        const float y_pos = (i+0.5)*widget_height + 0.3*font_scale*font->font_height_px;

        AddTextToHostBuffer(toUtf32(w.text), text_pad, y_pos, host_vbo_pos, host_vbo_index);

        //            if(w.widget_type == WidgetType::slider)
        if(!w.value.empty())
        {
            const auto utf32 = toUtf32(w.value);
            const float width = TextWidthPix(utf32);
            AddTextToHostBuffer(utf32, widget_width - text_pad - width, y_pos, host_vbo_pos, host_vbo_index);
        }
    }

    vbo_chars_pos = pangolin::GlBuffer( pangolin::GlArrayBuffer, host_vbo_pos );
    vbo_chars_index = pangolin::GlBuffer( pangolin::GlArrayBuffer, host_vbo_index );
    vao_chars.AddVertexAttrib(pangolin::DEFAULT_LOCATION_POSITION, vbo_chars_pos);
    vao_chars.AddVertexAttrib(GLSL_LOCATION_CHAR_INDEX, vbo_chars_index);

    vbo_chars_index.Unbind();
    vao_chars.Unbind();
}

void GlWidgetLayer::renderIntoRegion(const RenderParams& p)
{
    // ScopedGlEnable en_scissor(GL_SCISSOR_TEST);
    ScopedGlDisable dis_depth(GL_DEPTH_TEST);
    glEnable(GL_BLEND); // Assume this is okay to keep
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    setGlViewport(p.region);
    setGlScissor(p.region);
    glClearColor(0.0,0.0,0.0,1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto size = p.region.range();

    T_cm = ProjectionMatrixOrthographic(-0.5, size[0]-0.5, size[1]-0.5 - scroll_offset, -0.5 - scroll_offset, -1.0, 1.0);

    // TODO: try to do this less. It's expensive
    {
        UpdateWidgetParams();
        UpdateWidgetVBO(size[0]);
        UpdateCharsVBO(size[0]);
    }

    prog_widget.Bind();
    prog_widget.SetUniform("u_T_cm", T_cm);
    vao_widgets.Bind();
    glDrawArrays(GL_POINTS, 0, vbo_widgets.num_elements);
    prog_widget.Unbind();
    vao_widgets.Unbind();

    prog_text.Bind();
    prog_text.SetUniform("u_T_cm", T_cm);
    vao_chars.Bind();
    glActiveTexture(GL_TEXTURE0);
    font->mTex.Bind();
    glActiveTexture(GL_TEXTURE1);
    font_offsets.Bind();
    glDrawArrays(GL_POINTS, 0, vbo_chars_index.num_elements);
    prog_text.Unbind();
    vao_chars.Unbind();
    font->mTex.Unbind();

    glActiveTexture(GL_TEXTURE0);
}

// void GlWidgetLayer::Mouse(View&, MouseButton button, int x, int y, bool pressed, int button_state)
// {
//     if(button == MouseButtonLeft) {
//         auto w = WidgetXY(x,y);
//         if(selected_widget >= 0) {
//             auto& sw = widgets[selected_widget];
//             SetValue(x,y, pressed, false);
//             if(!pressed) {
//                 selected_widget = -1;
//             }
//         }else {
//             selected_widget = w.first;
//             SetValue(x,y, pressed, false);
//         }
//     }else if(button == MouseWheelUp || button == MouseWheelDown) {
//         const float delta = (button == MouseWheelDown ? 1.0f : -1.0f) * 0.25f*widget_height;
//         const float offset_max = (widgets.size()-1.0f) * widget_height;
//         scroll_offset = std::clamp(scroll_offset + delta, -offset_max, 0.0f);
//     }
// }

// void GlWidgetLayer::MouseMotion(View&, int x, int y, int button_state)
// {
//     SetValue(x,y, true, true);
// }

// void GlWidgetLayer::PassiveMouseMotion(View&, int x, int y, int button_state)
// {
//     auto w = WidgetXY(x,y);
//     hover_widget = (0 <= w.second.x() && w.second.x() < v.w) ? w.first : -1;
//     UpdateWidgetVBO();
//     UpdateCharsVBO();
// }

// void GlWidgetLayer::Special(View&, InputSpecial inType, float x, float y, float p1, float p2, float p3, float p4, int button_state)
// {
//     if(inType == InputSpecialScroll) {
//         const float delta = p2;
//         const float offset_max = (widgets.size()-1.0f) * widget_height;
//         scroll_offset = std::clamp(scroll_offset + delta, -offset_max, 0.0f);
//     }
// }

// void GlWidgetLayer::SetValue(float x, float y, bool pressed, bool dragging)
// {
//     auto w = WidgetXY(x,y);
//     if(w.first == selected_widget && 0 <= w.first && w.first < widgets.size()) {
//         WidgetParams& wp = widgets[w.first];
//         if(wp.widget_type==WidgetType::checkbox) {
//             if(pressed && !dragging) wp.value_percent = 1.0 - wp.value_percent;
//         }else if(wp.widget_type==WidgetType::button) {
//             wp.value_percent = pressed ? 0.0 : 1.0;
//         }else{
//             if( pressed || dragging) {
//                 const float val = std::clamp( (w.second.x() - widget_padding) / (v.w - 2*widget_padding), 0.0f, 1.0f);

//                 if( wp.divisions == 0) {
//                     // continuous version
//                     wp.value_percent = val;
//                 }else{
//                     // springy discrete version
//                     const float d = (std::round(val*wp.divisions)/wp.divisions);
//                     float diff = val - d;
//                     wp.value_percent = d + diff*sping_coeff;
//                 }
//             }else if(!pressed) {
//                 // de-springify
//                 if(wp.divisions) {
//                     wp.value_percent = (std::round(wp.value_percent*wp.divisions)/wp.divisions);
//                 }
//             }
//         }

//         if( wp.read_params) {
//             wp.read_params(wp);
//         }

//         dirty = true;
//     }
// }

// std::pair<int,Eigen::Vector2f> GlWidgetLayer::WidgetXY(float x, float y)
// {
//     const Eigen::Vector2f p_view(x - v.l, v.h - (y - v.b) - scroll_offset);
//     const int i = std::floor(p_view[1] / widget_height);
//     const Eigen::Vector2f p_widget(p_view[0], std::fmod(p_view[1], widget_height));
//     return {i, p_widget};
// }

void GlWidgetLayer::process_var_event(const pangolin::VarState::Event& event)
{
    using namespace pangolin;

    if(event.action == VarState::Event::Action::Added) {
        auto var = event.var;

        if( !strcmp(var->TypeId(), typeid(bool).name()) ) {
            widgets.push_back(WidgetParams{
                                           event.var->Meta().friendly, "", 1.0f, 0,
                                           var->Meta().flags & META_FLAG_TOGGLE ? WidgetType::checkbox : WidgetType::button,
                                           [var](const WidgetParams& p){ // read_params
                                               Var<bool> v(var);
                                               v = p.value_percent > 0.5;
                                               v.Meta().gui_changed = true;
                                           },
                                           [var](WidgetParams& p){ // write params
                                               Var<bool> v(var);
                                               p.value_percent = v ? 1.0 : 0.0;
                                               },
                                           });
        } else if (!strcmp(var->TypeId(), typeid(double).name()) ||
                   !strcmp(var->TypeId(), typeid(float).name()) ||
                   !strcmp(var->TypeId(), typeid(size_t).name()) ||
                   !strcmp(var->TypeId(), typeid(int8_t).name()) ||
                   !strcmp(var->TypeId(), typeid(uint8_t).name()) ||
                   !strcmp(var->TypeId(), typeid(int16_t).name()) ||
                   !strcmp(var->TypeId(), typeid(uint16_t).name()) ||
                   !strcmp(var->TypeId(), typeid(int32_t).name()) ||
                   !strcmp(var->TypeId(), typeid(uint32_t).name()) ||
                   !strcmp(var->TypeId(), typeid(int64_t).name()) ||
                   !strcmp(var->TypeId(), typeid(uint64_t).name())
                   )
        {
            const bool is_integral = strcmp(var->TypeId(), typeid(double).name()) &&
                                     strcmp(var->TypeId(), typeid(float).name());

            auto& r = var->Meta().range;
            const double range = r[1]-r[0];
            const double steps = is_integral ? (range / var->Meta().increment) : 0.0;
            widgets.push_back(WidgetParams{
                                           event.var->Meta().friendly, "",
                                           1.0f, static_cast<int>(steps),
                                           WidgetType::slider,
                                           [var](const WidgetParams& p){ // read_params
                                               Var<double> v(var);
                                               auto& r = var->Meta().range;
                                               const double range = r[1]-r[0];
                                               v = std::clamp(r[0] + range*p.value_percent, r[0], r[1]);
                                               v.Meta().gui_changed = true;
                                           },
                                           [var](WidgetParams& p){ // write params
                                               // TODO: this is breaking the 'springyness' for integral sliders
                                               // because reading from the int value is flooring the value_percent.
                                               Var<double> v(var);
                                               auto& r = var->Meta().range;
                                               const double range = r[1]-r[0];
                                               p.value_percent = std::clamp( (v-r[0]) / range, 0.0, 1.0);
                                               p.value = var->str->Get();
                                               },
                                           });
            widgets.back().write_params(widgets.back());
        } else if (!strcmp(var->TypeId(), typeid(std::function<void(void)>).name() ) ) {
            widgets.push_back(WidgetParams{
                event.var->Meta().friendly, "",
                1.0f, 0, WidgetType::button,
                [var](const WidgetParams& p){ // read_params
                    Var<std::function<void(void)>> v(var);
                    if(p.value_percent > 0.5) v.Get()();
                },{}
            });
        }else if(var->str){
            widgets.push_back(WidgetParams{
                                           event.var->Meta().friendly, "",
                                           1.0f, 0, WidgetType::textbox,
                                           [var](const WidgetParams& p){ // read_params
                                           },
                                           [var](WidgetParams& p){ // write params
                                               Var<std::string> v(var);
                                               p.value = v.Get();
                                               },
                                           });
        }

        dirty = true;
    }else if(event.action == pangolin::VarState::Event::Action::Removed){
        // TODO: should remove the widget here
    }
}

PANGO_CREATE(WidgetLayer) {
    return Shared<GlWidgetLayer>::make(p);
}

}
