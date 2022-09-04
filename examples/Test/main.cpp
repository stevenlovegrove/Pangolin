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
#include "backtrace.h"

#include <locale>
#include <codecvt>

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

pangolin::ManagedImage<Eigen::Vector4f> MakeFontLookupImage(const pangolin::GlFont& font)
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

pangolin::ManagedImage<uint16_t> MakeFontIndexImage(pangolin::GlFont& font, const std::string& utf8)
{
    const std::u32string utf32 = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.from_bytes(utf8);

    pangolin::ManagedImage<uint16_t> img(utf32.size(), 1);

    for(size_t i=0; i < utf32.size(); ++i) {
        const auto& ch = font.chardata[utf32[i]];
        img(i,0) = ch.AtlasIndex();
    }

    return img;
}

template<typename T>
pangolin::GlTexture TextureFromImage(const pangolin::Image<T>& img)
{
    pangolin::GlTexture tex;
    using Fmt = pangolin::GlFormatTraits<T>;
    CheckGlDieOnError();
    tex.Reinitialise(img.w, img.h, Fmt::glinternalformat, false, 0, Fmt::glformat, Fmt::gltype, img.ptr);
    CheckGlDieOnError();
    return tex;
}

void MainSliderExperiments()
{
//    PrintBacktrace();
//    exit(0);

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
    std::cerr << "1" << std::endl;
    GlTexture font_offsets = TextureFromImage(MakeFontLookupImage(font));
    std::cerr << "2" << std::endl;
    GlTexture text = TextureFromImage(MakeFontIndexImage(font, "Hello"));
    std::cerr << "3" << std::endl;


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
            prog.SetUniform("u_text", 2);
            prog.SetUniform("u_mouse_pos", (float)handler.x, (float)handler.y);

            auto& co = font.chardata[handler.last_codepoint];
            prog.SetUniform("u_char_id", (int)co.AtlasIndex());

            vao.Bind();
            glActiveTexture(GL_TEXTURE0);
            font.mTex.Bind();
            glActiveTexture(GL_TEXTURE1);
            font_offsets.Bind();
            glActiveTexture(GL_TEXTURE2);
            text.Bind();
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
