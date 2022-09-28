#pragma once

#include <pangolin/display/default_font.h>
#include <pangolin/display/display.h>
#include <pangolin/display/widgets.h>
#include <pangolin/utils/file_utils.h>
#include <pangolin/gl/glsl.h>
#include <pangolin/var/var.h>
#include <pangolin/var/varextra.h>
#include <locale>
#include <string>
#include <codecvt>

namespace pangolin
{

struct WidgetPanel : public View, public Handler
{
    static constexpr float sping_coeff = 0.2;

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
        std::string value;
        float value_percent;
        int divisions;
        WidgetType widget_type;
        std::function<void(const WidgetParams&)> read_params;
        std::function<void(WidgetParams&)> write_params;
    };


    WidgetPanel();

    void LoadShaders();

    void UpdateWidgetParams();

    void UpdateWidgetVBO();

    std::u32string toUtf32(const std::string& utf8);

    float TextWidthPix(const std::u32string& utf32);

    void AddTextToHostBuffer(
        const std::u32string& utf32, float x, float y,
        std::vector<Eigen::Vector3f>& host_vbo_pos,
        std::vector<uint16_t>& host_vbo_index);

    void UpdateCharsVBO();

    void Render() override;

    void Resize(const Viewport& p) override;

    void Keyboard(View&, unsigned char key, int x, int y, bool pressed) override;

    void Mouse(View&, MouseButton button, int x, int y, bool pressed, int button_state) override;

    void MouseMotion(View&, int x, int y, int button_state) override;

    void PassiveMouseMotion(View&, int x, int y, int button_state) override;

    void Special(View&, InputSpecial inType, float x, float y, float p1, float p2, float p3, float p4, int button_state) override;

    void SetValue(float x, float y, bool pressed, bool dragging);

    std::pair<int,Eigen::Vector2f> WidgetXY(float x, float y);

    void process_var_event(const pangolin::VarState::Event& event);

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
