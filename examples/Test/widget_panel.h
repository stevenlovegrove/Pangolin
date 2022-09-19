#pragma once

#include <pangolin/display/display.h>
#include <pangolin/display/widgets.h>
#include <pangolin/utils/file_utils.h>
#include <pangolin/gl/glsl.h>
#include <pangolin/var/var.h>
#include <pangolin/var/varextra.h>
#include <string>
#include <codecvt>

namespace pangolin
{

struct WidgetPanel : public View, public Handler
{
    enum class WidgetType
    {
        label = 0,
        textbox,
        button,
        checkbox,
        slider,
        seperator
    };

    struct WidgetParams
    {
        std::string text;
        float value_percent;
        int divisions;
        WidgetType widget_type;
        std::function<void(const WidgetParams&)> read_params;
        std::function<void(WidgetParams&)> write_params;
    };


    WidgetPanel()
        : widget_width(400.0),
          widget_height(70.0),
          widget_padding(10.0),
          font_scale(0.7),
          scroll_offset(0.0),
          selected_widget(-1)
    {
        this->SetHandler(this);

        font = std::make_unique<pangolin::GlFont >(
            "/Users/stevenlovegrove/code/msdf-atlas-gen/fonts/AnonymousPro.ttf_map.png",
            "/Users/stevenlovegrove/code/msdf-atlas-gen/fonts/AnonymousPro.ttf_map.json"
            );
        font->InitialiseGlTexture();
        font_offsets.Load(font->MakeFontLookupImage());

        const std::string shader_dir = "/components/pango_opengl/shaders/";
        const std::string shader_widget = shader_dir + "main_widgets.glsl";
        const std::string shader_text = shader_dir + "main_text.glsl";

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

        // Receive Pangolin var events
        sigslot_lifetime = pangolin::VarState::I().RegisterForVarEvents(
            [this](const pangolin::VarState::Event& event){
                process_var_event(event);
            }, true
            );
    }

    void UpdateWidgetParams()
    {
        for(auto& wp : widgets) {
            if(wp.write_params) {
                wp.write_params(wp);
            }
        }
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
            host_vbo.emplace_back(0.0, i, w.divisions + w.value_percent/2.0, uint(w.widget_type) );
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
        prog_text.SetUniform("u_font_bitmap_type", static_cast<int>(font->bitmap_type) );
        prog_text.SetUniform("u_scale", font_scale);
        prog_text.SetUniform("u_max_sdf_dist_uv", font->bitmap_max_sdf_dist_uv[0], font->bitmap_max_sdf_dist_uv[1] );
        prog_text.SetUniform("u_color", 0.0f, 0.0f, 0.0f);

        std::vector<Eigen::Vector3f> host_vbo_pos;
        std::vector<uint16_t> host_vbo_index;
        for(int i=0; i < widgets.size(); ++i) {
            const auto& w = widgets[i];
            const std::string utf8 = w.text;
            const std::u32string utf32 = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.from_bytes(utf8);
            const std::u16string index16 = font->to_index_string(utf32);
            float adv = 2.5*widget_padding;
            GlFont::codepoint_t last_char = 0;

            // y-position is roughly center with fudge factor since text is balanced low.
            const float y_pos = (i+0.5)*widget_height + 0.3*font_scale*font->font_height_px;

            for(int c=0; c < index16.size(); ++c) {
                const GlFont::codepoint_t this_char = utf32[c];

                if(!index16[c]) {
                    // TODO: use some symbol such as '?' maybe
                    adv += font_scale * font->default_advance_px;
                    last_char = 0;
                }else{
                    auto ch = font->chardata[this_char];

                    if(last_char) {
                        const auto key = GlFont::codepointpair_t(last_char,this_char);
                        const auto kit = font->kern_table.find(key);
                        const float kern = (kit != font->kern_table.end()) ? kit->second : 0;
                        adv += font_scale * kern;
                    }

                    host_vbo_pos.emplace_back(adv, y_pos, 0.0 );
                    host_vbo_index.emplace_back(index16[c]);
                    adv += font_scale * ch.StepX();
                    last_char = this_char;
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

//        if(Pushed(dirty))
        {
//            UpdateWidgetParams();
            UpdateWidgetVBO();
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
        if(selected_widget >= 0) {
            auto& sw = widgets[selected_widget];
            SetValue(x,y, pressed, false);


            if(!pressed) {


                selected_widget = -1;
            }
        }else {
            selected_widget = w.first;
            SetValue(x,y, pressed, false);
        }
    }

    void MouseMotion(View&, int x, int y, int button_state) override
    {
        SetValue(x,y, true, true);
    }

    void PassiveMouseMotion(View&, int x, int y, int button_state) override
    {
        auto w = WidgetXY(x,y);
        hover_widget = (0 <= w.second.x() && w.second.x() < widget_width) ? w.first : -1;
        UpdateWidgetVBO();
        UpdateCharsVBO();
    }

    void Special(View&, InputSpecial inType, float x, float y, float p1, float p2, float p3, float p4, int button_state) override
    {
        if(inType == InputSpecialScroll) {
            scroll_offset = std::clamp(scroll_offset + p2, -(float)((widgets.size()-1)*widget_height), 0.0f);
        }
    }

    void SetValue(float x, float y, bool pressed, bool dragging)
    {
        auto w = WidgetXY(x,y);
        if(w.first == selected_widget && 0 <= w.first && w.first < widgets.size()) {
            WidgetParams& wp = widgets[w.first];
            if(wp.widget_type==WidgetType::checkbox) {
                if(pressed && !dragging) wp.value_percent = 1.0 - wp.value_percent;
            }else if(wp.widget_type==WidgetType::button) {
                wp.value_percent = pressed ? 0.0 : 1.0;
            }else{
                if( pressed || dragging) {
                    const float val = std::clamp( (w.second.x() - widget_padding) / (widget_width - 2*widget_padding), 0.0f, 1.0f);

                    if( wp.divisions == 0) {
                        // continuous version
                        wp.value_percent = val;
                    }else{
                        // springy discrete version
                        const float d = (std::round(val*wp.divisions)/wp.divisions);
                        float diff = val - d;
                        wp.value_percent = d + diff*0.2;
                    }
                }else if(!pressed) {
                    // de-springify
                    if(wp.divisions) {
                        wp.value_percent = (std::round(wp.value_percent*wp.divisions)/wp.divisions);
                    }
                }
            }

            if( wp.read_params) {
                wp.read_params(wp);
            }

            dirty = true;
        }
    }

    std::pair<int,Eigen::Vector2f> WidgetXY(float x, float y)
    {
        const Eigen::Vector2f p_view(x - v.l, v.h - (y - v.b) - scroll_offset);
        const int i = std::floor(p_view[1] / widget_height);
        const Eigen::Vector2f p_widget(p_view[0], std::fmod(p_view[1], widget_height));
        return {i, p_widget};
    }

    void process_var_event(const pangolin::VarState::Event& event)
    {
        using namespace pangolin;

        if(event.action == VarState::Event::Action::Added) {
            auto var = event.var;

            if( !strcmp(var->TypeId(), typeid(bool).name()) ) {
                widgets.push_back(WidgetParams{
                    event.var->Meta().friendly, 1.0f, 0,
                    var->Meta().flags & META_FLAG_TOGGLE ? WidgetType::checkbox : WidgetType::button
                });
            } else if (!strcmp(var->TypeId(), typeid(double).name()) ||
                       !strcmp(var->TypeId(), typeid(float).name()) ||
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
                auto& r = var->Meta().range;
                const double range = r[1]-r[0];
                const double steps = range / var->Meta().increment;
                widgets.push_back(WidgetParams{
                    event.var->Meta().friendly,
                    1.0f, static_cast<int>(steps),
                    WidgetType::slider,
                    [var](const WidgetParams& p){ // read_params
                        Var<double> v(var);
                        auto& r = var->Meta().range;
                        const double range = r[1]-r[0];
                        v = r[0] + range*p.value_percent;
                    },
                    [var](WidgetParams& p){ // write params
                        // TODO: this is breaking the 'springyness' for integral sliders
                        // because reading from the int value is flooring the value_percent.
                        Var<double> v(var);
                        auto& r = var->Meta().range;
                        const double range = r[1]-r[0];
                        p.value_percent = (v-r[0]) / range;
                    },
                });
                widgets.back().write_params(widgets.back());
            } else if (!strcmp(var->TypeId(), typeid(std::function<void(void)>).name() ) ) {
                widgets.push_back(WidgetParams{
                    event.var->Meta().friendly,
                    1.0f, 0, WidgetType::button,
                    [var](const WidgetParams& p){ // read_params
                        Var<std::function<void(void)>> v(var);
                        if(p.value_percent > 0.5) v.Get()();
                    }
                });
            }else if(var->str){
                widgets.push_back(WidgetParams{
                    event.var->Meta().friendly,
                    1.0f, 0, WidgetType::textbox,
                    [var](const WidgetParams& p){ // read_params
                    },
                    [var](WidgetParams& p){ // write params
                        Var<std::string> v(var);
                        p.text = v.Get();
                    },
                });
            }

            dirty = true;
        }else if(event.action == pangolin::VarState::Event::Action::Removed){
            // TODO: should remove the widget here
        }
    }

    bool dirty;

    std::shared_ptr<GlFont> font;
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

    float widget_width;
    float widget_height;
    float widget_padding;
    float font_scale;

    float scroll_offset;

    int selected_widget;
    int hover_widget;
    std::vector<WidgetParams> widgets;

    sigslot::connection sigslot_lifetime;
};

}
