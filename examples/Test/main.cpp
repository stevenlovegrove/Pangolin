#include <pangolin/display/display.h>
#include <pangolin/display/default_font.h>
#include <pangolin/var/var.h>
#include <pangolin/display/widgets.h>

#include <pangolin/gl/gldraw.h>
#include <pangolin/gl/glvbo.h>
#include <pangolin/gl/glsl.h>
#include <pangolin/gl/glfont.h>
#include "notifier.h"
#include "pangolin/var/varextra.h"

#include <locale>
#include <codecvt>

const char* shader_text = R"Shader(
@start vertex
//#version 150 core

attribute vec2 a_position;
attribute vec2 a_texcoord;
uniform vec2 u_scale;
uniform vec2 u_offset;
varying vec2 v_texcoord;
void main() {
    gl_Position = vec4(u_scale * (a_position + u_offset) * 2.0 - 1.0, 0.0, 1.0);
    v_texcoord = a_texcoord;
}

@start fragment
//#version 150 core

varying vec2 v_texcoord;
uniform sampler2D u_texture;
uniform vec4 u_color_fg;
uniform vec4 u_color_bg;

const float pxRange = 2.0;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

float screenPxRange() {
//    vec2 unitRange = vec2(pxRange)/vec2(textureSize(u_texture, 0));
    vec2 unitRange = vec2(pxRange)/vec2(514,514);
    vec2 screenTexSize = vec2(1.0)/fwidth(v_texcoord);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

void main() {
  vec4 sample = texture2D(u_texture, v_texcoord);
  vec3 msd = sample.xyz;
  float sd = median(msd.r, msd.g, msd.b);
  float screenPxDistance = screenPxRange()*(sd - 0.5);
  float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);

  gl_FragColor = mix(u_color_bg, u_color_fg, opacity);
}
)Shader";

struct HoverHandler : public pangolin::Handler
{
    void Mouse(pangolin::View& d, pangolin::MouseButton button, int x, int y, bool pressed, int button_state) override
    {
        this->x = x;
        this->y = y;
    }

    void MouseMotion(pangolin::View&, int x, int y, int button_state)  override
    {
        this->x = x;
        this->y = y;
    }

    void Keyboard(pangolin::View&, unsigned char key, int x, int y, bool pressed) override
    {
        if(pressed) last_codepoint = key;
    }


    uint32_t last_codepoint;
    int x,y;

};

void MainRenderTextWithNewAtlas()
{
    using namespace pangolin;

    //    pangolin::CreateWindowAndBind("Pango GL Triangle With VBO and Shader", 500, 500, {{PARAM_GL_PROFILE, "3.2 CORE"}});
    pangolin::CreateWindowAndBind("Pango GL Triangle With VBO and Shader", 500, 500, {{PARAM_GL_PROFILE, "LEGACY"}});
    CheckGlDieOnError();

    pangolin::GlFont font("/Users/stevenlovegrove/code/msdf-atlas-gen/fonts/AnonymousPro.ttf_map.png", "/Users/stevenlovegrove/code/msdf-atlas-gen/fonts/AnonymousPro.ttf_map.json");

    CheckGlDieOnError();

    pangolin::GlSlProgram prog_text;
    prog_text.AddShader( pangolin::GlSlAnnotatedShader, shader_text );
    prog_text.BindPangolinDefaultAttribLocationsAndLink();

    float scale = 1.0;
    float dx = 0.0;

    RegisterKeyPressCallback('=', [&](){scale *= 1.1;});
    RegisterKeyPressCallback('-', [&](){scale /= 1.1;});

    while( !pangolin::ShouldQuit() )
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        dx += 0.01;

        {
            auto& v = DisplayBase().v;

            prog_text.Bind();
            prog_text.SetUniform("u_scale",  scale / v.w, scale / v.h);
            prog_text.SetUniform("u_color_fg", Colour::White() );
            prog_text.SetUniform("u_color_bg", Colour::Black().WithAlpha(0.0) );
            prog_text.SetUniform("u_offset", 10.0f + dx, 10.0f );
            font.Text("Test").DrawGlSl();
            prog_text.Unbind();
        }

        pangolin::FinishFrame();
    }
}

pangolin::ManagedImage<Eigen::Vector4f> MakeFontLookupImage(pangolin::GlFont& font)
{
    pangolin::ManagedImage<Eigen::Vector4f> img(font.chardata.size(), 2);

    for(const auto& cp_char : font.chardata) {
        img(cp_char.second.AtlasIndex(), 0) = {
            cp_char.second.GetVert(0).tu,
            cp_char.second.GetVert(0).tv,
            cp_char.second.GetVert(2).tu - cp_char.second.GetVert(0).tu, // w
            cp_char.second.GetVert(2).tv - cp_char.second.GetVert(0).tv  // h
        };
        img(cp_char.second.AtlasIndex(), 1) = {
            cp_char.second.GetVert(0).x, cp_char.second.GetVert(0).y, 0.0, 0.0
        };
    }

    return img;
}

void MainSliderExperiments()
{
    using namespace pangolin;

    pangolin::CreateWindowAndBind("Pango GL Triangle With VBO and Shader", 500, 500, {{PARAM_GL_PROFILE, "3.2 CORE"}});
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    HoverHandler handler;
    DisplayBase().SetHandler(&handler);

    auto& v = DisplayBase().v;

    pangolin::GlBuffer vbo(pangolin::GlArrayBuffer,
                           std::vector<Eigen::Vector3f>{
                               { 0.0f, 0.0f, 0.0f},
                               { v.w/2.0,  0.0f, 0.0f },
                               { 0.0f, v.h, 0.0f },
                               { v.w/2.0,  v.h, 0.0f }
                           }
                           );

    pangolin::GlFont font("/Users/stevenlovegrove/code/msdf-atlas-gen/fonts/AnonymousPro.ttf_map.png", "/Users/stevenlovegrove/code/msdf-atlas-gen/fonts/AnonymousPro.ttf_map.json");
    font.InitialiseGlTexture();
    GlTexture font_offsets;
    auto img = MakeFontLookupImage(font);
    {
        font_offsets.Reinitialise(img.w, img.h, GL_RGBA32F, false, 0, GL_RGBA, GL_FLOAT, img.ptr);
    }


//    GlTexture matcap;
//    matcap.LoadFromFile("/Users/stevenlovegrove/Downloads/matcap1.png");
    //    matcap.LoadFromFile("/Users/stevenlovegrove/Downloads/matcap3.jpg");
    //    matcap.LoadFromFile("/Users/stevenlovegrove/Downloads/matcap_normal.jpg");

    std::string shader_dir = pangolin::FindPath(GetExecutableDir(), "/Pangolin/examples/Test/shaders/");
    std::string shader = shader_dir + "my_shader.glsl";


    pangolin::GlSlProgram prog;
    prog.AddShaderFromFile(pangolin::GlSlAnnotatedShader, shader, {}, {shader_dir});
    prog.BindPangolinDefaultAttribLocationsAndLink();


    GlVertexArrayObject vao;
    vao.AddVertexAttrib(pangolin::DEFAULT_LOCATION_POSITION, vbo);
    vao.Unbind();

    bool reload_shader = false;

    FileNotifier notifier(shader, [&](){
        try {
            reload_shader = true;
        }catch(const std::exception& e) {
            std::cout << e.what() << std::endl;
        }}
    );

    DisplayBase().Activate();

    auto T_cm = ProjectionMatrixOrthographic(-0.5, v.w-0.5, -0.5, v.h-0.5, -1.0, 1.0);

    while( !pangolin::ShouldQuit() )
    {
        if(pangolin::Pushed(reload_shader)) {
            prog.ReloadShaderFiles();
        }

        glClearColor(0.7, 0.7, 0.7, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(true) {
            prog.Bind();

            prog.SetUniform("u_T_cm", T_cm);
            prog.SetUniform("u_val", std::clamp((handler.x - 15.0f)/400.0f, 0.0f, 1.0f ) );
            prog.SetUniform("u_font_atlas", 0);
            prog.SetUniform("u_font_offsets", 1);
            prog.SetUniform("u_mouse_pos", (float)handler.x, (float)handler.y);

            auto& co = font.chardata[handler.last_codepoint];
            prog.SetUniform("u_char_id", (int)co.AtlasIndex());

            vao.Bind();
            glActiveTexture(GL_TEXTURE0);
            font.mTex.Bind();
            glActiveTexture(GL_TEXTURE1);
            font_offsets.Bind();
            glDrawArrays(GL_TRIANGLE_STRIP, 0, vbo.num_elements);
            prog.Unbind();
            vao.Unbind();
        }

        pangolin::FinishFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main( int /*argc*/, char** /*argv*/ )
{
    MainSliderExperiments();
    //    MainRenderTextWithNewAtlas();
    return 0;
}
