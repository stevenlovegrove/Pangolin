#pragma once

#include <pangolin/display/display.h>
#include <pangolin/display/widgets.h>
#include <pangolin/utils/file_utils.h>
#include <pangolin/gl/glsl.h>
#include <pangolin/var/var.h>
#include <pangolin/var/varextra.h>

#include "notifier.h"
#include "text.h"

namespace pangolin
{

struct WidgetPanel : public View ,public Handler
{
    enum class WidgetType
    {
        label = 0,
        textbox,
        button,
        slider
    };

    struct WidgetParams
    {
        std::string text;
        float value_percent;
        WidgetType widget_type;
    };


    WidgetPanel()
        : notifier([this](){dirty=true;}),
          widget_width(400.0),
          widget_height(70.0),
          widget_padding(10.0),
          font_scale(0.7),
          scroll_offset(0.0),
          selected_widget(-1)
    {
        this->SetHandler(this);
        font = std::make_unique<pangolin::GlFont >("/Users/stevenlovegrove/code/msdf-atlas-gen/fonts/AnonymousPro.ttf_map.png", "/Users/stevenlovegrove/code/msdf-atlas-gen/fonts/AnonymousPro.ttf_map.json");
        font->InitialiseGlTexture();
        font_offsets = TextureFromImage(MakeFontLookupImage(*font));

        const std::string shader_dir = pangolin::FindPath(GetExecutableDir(), "/Pangolin/examples/Test/shaders/");
        const std::string shader_widget = shader_dir + "main_widgets.glsl";
        const std::string shader_text = shader_dir + "main_text.glsl";

        widgets.emplace_back(WidgetParams{"Button", 0.0f, WidgetType::button});
        widgets.emplace_back(WidgetParams{"Button2", 1.0f, WidgetType::button});
        widgets.emplace_back(WidgetParams{"Some Label", 1.0f, WidgetType::label});
        widgets.emplace_back(WidgetParams{"Some TextBox", 1.0f, WidgetType::textbox});

        for(int i=0; i < 5; ++i) {
            widgets.emplace_back(WidgetParams{"Widget" + std::to_string(i), i * 0.1f, WidgetType::slider});
        }
        widgets.emplace_back(WidgetParams{"-------------------", 1.0f, WidgetType::label});// spacer
        for(int i=5; i < 10; ++i) {
            widgets.emplace_back(WidgetParams{"Widget" + std::to_string(i), i * 0.1f, WidgetType::slider});
        }

        notifier.AddPaths({shader_widget, shader_text});
        CheckGlDieOnError();

        prog_widget.AddShaderFromFile(pangolin::GlSlAnnotatedShader, shader_widget, {}, {shader_dir});
        glBindAttribLocation(prog_widget.ProgramId(), DEFAULT_LOCATION_POSITION, DEFAULT_NAME_POSITION);
        prog_widget.Link();
        UpdateWidgetVBO();
        CheckGlDieOnError();

        prog_text.AddShaderFromFile(pangolin::GlSlAnnotatedShader, shader_text, {}, {shader_dir});
        glBindAttribLocation(prog_text.ProgramId(), DEFAULT_LOCATION_POSITION, DEFAULT_NAME_POSITION);
        prog_text.Link();
        UpdateCharsVBO();
        CheckGlDieOnError();
    }

    void UpdateWidgetVBO()
    {
        prog_widget.Bind();
        prog_widget.SetUniform("u_val", std::clamp(0.5f, 0.0f, 1.0f ) );
        prog_widget.SetUniform("u_width",  widget_width );
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

        std::vector<Eigen::Vector4f> host_vbo;
        for(int i=0; i < widgets.size(); ++i) {
            const auto& w = widgets[i];
            host_vbo.emplace_back(0.0, i, w.value_percent, uint(w.widget_type) );
        }

        vbo_widgets = pangolin::GlBuffer( pangolin::GlArrayBuffer, host_vbo );
        vao_widgets.AddVertexAttrib(pangolin::DEFAULT_LOCATION_POSITION, vbo_widgets);
        vao_widgets.Unbind();
    }

    void UpdateCharsVBO()
    {
        prog_text.Bind();
        prog_text.SetUniform("u_font_atlas", 0);
        prog_text.SetUniform("u_font_offsets", 1);
        prog_text.SetUniform("u_scale", font_scale);

        std::vector<Eigen::Vector3f> host_vbo_pos;
        std::vector<uint16_t> host_vbo_index;
        for(int i=0; i < widgets.size(); ++i) {
            const auto& w = widgets[i];
            const std::string utf8 = w.text;
            const std::u32string utf32 = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.from_bytes(utf8);
            const std::u16string index16 = to_index_string(*font, utf32);
            float adv = 2.5*widget_padding;
            for(int c=0; c < index16.size(); ++c) {
                if(c > 0) {
                    const auto key = GlFont::codepointpair_t(utf32[c-1],utf32[c]);
                    const auto kit = font->kern_table.find(key);
                    adv += font_scale * ((kit != font->kern_table.end()) ? kit->second : font->font_max_width_px);
                }
                if(utf32[c] != ' ') {
//                    host_vbo_pos.emplace_back(adv, (i+0.35)*widget_height + 3.0*(w.widget_type == WidgetType::button && w.value_percent > 0.5), 0.0 ); // 'pressing'
                    host_vbo_pos.emplace_back(adv, (i+0.35)*widget_height, 0.0 );
                    host_vbo_index.emplace_back(index16[c]);
                }
            }
        }

        vbo_chars_pos = pangolin::GlBuffer( pangolin::GlArrayBuffer, host_vbo_pos );
        vbo_chars_index = pangolin::GlBuffer( pangolin::GlArrayBuffer, host_vbo_index );
        vao_chars.AddVertexAttrib(pangolin::DEFAULT_LOCATION_POSITION, vbo_chars_pos);
        vao_chars.AddVertexAttrib(1, vbo_chars_index);
        vao_chars.Unbind();
    }

    void Render() override
    {
        T_cm = ProjectionMatrixOrthographic(-0.5, v.w-0.5, v.h-0.5 - scroll_offset, -0.5 - scroll_offset, -1.0, 1.0);

        if(Pushed(dirty)) {
            prog_widget.ReloadShaderFiles();
            UpdateWidgetVBO();
            prog_text.ReloadShaderFiles();
            UpdateCharsVBO();
        }

        Activate();
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
    }

    void Resize(const Viewport& p) override
    {
        View::Resize(p);
    }

    void Keyboard(View&, unsigned char key, int x, int y, bool pressed) override
    {
    }

    void Mouse(View&, MouseButton button, int x, int y, bool pressed, int button_state) override
    {
        auto w = WidgetXY(x,y);
        selected_widget = pressed ? w.first : -1;
        SetValue(x,y);
    }

    void MouseMotion(View&, int x, int y, int button_state) override
    {
        SetValue(x,y);
    }

    void PassiveMouseMotion(View&, int x, int y, int button_state) override
    {
        auto w = WidgetXY(x,y);
        hover_widget = w.first;
        UpdateWidgetVBO();
        UpdateCharsVBO();
    }

    void Special(View&, InputSpecial inType, float x, float y, float p1, float p2, float p3, float p4, int button_state) override
    {
        if(inType == InputSpecialScroll) {
            scroll_offset = std::clamp(scroll_offset + p2, -(float)((widgets.size()-1)*widget_height), 0.0f);
        }
    }

    void SetValue(float x, float y)
    {
        auto w = WidgetXY(x,y);
        if(w.first == selected_widget && 0 <= w.first && w.first < widgets.size()) {
            const float x = std::clamp( (w.second.x() - widget_padding) / (widget_width - 2*widget_padding), 0.0f, 1.0f);
            WidgetParams& wp = widgets[w.first];
            wp.value_percent = x;
            UpdateWidgetVBO();
            UpdateCharsVBO();
        }
    }

    std::pair<int,Eigen::Vector2f> WidgetXY(float x, float y)
    {
        const Eigen::Vector2f p_view(x - v.l, v.h - (y - v.b) - scroll_offset);
        const int i = std::floor(p_view[1] / widget_height);
        const Eigen::Vector2f p_widget(p_view[0], std::fmod(p_view[1], widget_height));
        return {i, p_widget};
    }


    bool dirty;

    std::unique_ptr<GlFont> font;
    GlTexture font_offsets;

    OpenGlMatrix T_cm;
    pangolin::GlSlProgram prog_widget;
    pangolin::GlSlProgram prog_text;
    pangolin::GlBuffer vbo_widgets;
    GlVertexArrayObject vao_widgets;

    pangolin::GlBuffer vbo_chars_pos;
    pangolin::GlBuffer vbo_chars_index;
    GlVertexArrayObject vao_chars;

    bool reload_shader = false;
    FileNotifier notifier;

    float widget_width;
    float widget_height;
    float widget_padding;
    float font_scale;

    float scroll_offset;

    int selected_widget;
    int hover_widget;
    std::vector<WidgetParams> widgets;
};

}
