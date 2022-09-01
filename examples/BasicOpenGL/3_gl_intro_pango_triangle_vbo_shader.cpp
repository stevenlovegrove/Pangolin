#include <pangolin/display/display.h>
#include <pangolin/display/default_font.h>
#include <pangolin/var/var.h>
#include <pangolin/display/widgets.h>

#include <pangolin/gl/gldraw.h>
#include <pangolin/gl/glvbo.h>
#include <pangolin/gl/glsl.h>
#include <pangolin/gl/glfont.h>

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

const char* my_shader = R"Shader(
@start vertex
#version 150 core

in vec3 a_position;
out vec2 v_pos;
uniform mat4 u_T_cm;

void main() {
    gl_Position = u_T_cm * vec4(a_position, 1.0);
    v_pos = a_position.xy;
}

@start fragment
#version 150 core
in vec2 v_pos;
out vec4 FragColor;

const vec2 light_dir = vec2(-sqrt(0.5), -sqrt(0.5));

// x in interval [0, 2]
vec4 mix3(vec4 a, vec4 b, vec4 c, float x )
{
    float wa = 1.0 - clamp( x, 0.0, 1.0);
    float wb = 1.0 - clamp( abs(x-1.0), 0.0, 1.0);
    float wc = 1.0 - clamp( 2.0-x, 0.0, 1.0);
    return wa*a + wb*b + wc*c;
}

float opacity(float sdf)
{
    return clamp(-sdf + 0.5, 0.0, 1.0);
}

float sdf_circ(vec2 p, vec2 center, float rad)
{
    float dist = length(p - center);
    return dist - rad;
}

float sdf_rect(vec2 p, vec2 center, vec2 half_size) {
  vec2 d = abs(p - center) - half_size;
  float outside = length(max(d, 0.));
  float inside = min(max(d.x, d.y), 0.);
  return outside + inside;
}

float sdf_rounded_rect(vec2 p, vec2 center, vec2 half_size, float rad) {
    return sdf_rect(p,center,half_size-vec2(rad)) - rad;
}

void main() {
    float half_height = 20.0;
    float padding = 24.0;
    float rad = half_height * 0.5;
    vec2 v_pos2 = vec2(v_pos.x, mod(v_pos.y, 2*(padding+half_height) ) );

    float sdf = sdf_rounded_rect(v_pos2, vec2(padding+100.0, padding+half_height), vec2(100, half_height), rad);
    vec2 dsdf = vec2(dFdx(sdf), dFdy(sdf));
    dsdf /= length(dsdf);

//    FragColor = mix(vec4(0.9,0.9,0.9,1.0), vec4(1.0,1.0,1.0,1.0), opacity(sdf));
    FragColor = mix3(
            vec4(0.8,0.8,0.8,1.0),
            vec4(vec3(0.5, 0.5, 0.5) + dot(dsdf,light_dir) * vec3(0.5, 0.0, 0.0), 1.0),
            vec4(0.9,0.9,0.9,1.0),
            sdf);
}
)Shader";

void sample()
{
    using namespace pangolin;

    pangolin::CreateWindowAndBind("Pango GL Triangle With VBO and Shader", 500, 500, {{PARAM_GL_PROFILE, "3.2 CORE"}});
//    pangolin::CreateWindowAndBind("Pango GL Triangle With VBO and Shader", 500, 500, {{PARAM_GL_PROFILE, "LEGACY"}});
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

void sample2()
{
    using namespace pangolin;

    pangolin::CreateWindowAndBind("Pango GL Triangle With VBO and Shader", 500, 500, {{PARAM_GL_PROFILE, "3.2 CORE"}});
//    pangolin::CreateWindowAndBind("Pango GL Triangle With VBO and Shader", 500, 500, {{PARAM_GL_PROFILE, "LEGACY"}});
    CheckGlDieOnError();

    auto& v = DisplayBase().v;

    pangolin::GlBuffer vbo(pangolin::GlArrayBuffer,
        std::vector<Eigen::Vector3f>{
           { 0.0f, 0.0f, 0.0f},
           { v.w/2.0,  0.0f, 0.0f },
           { 0.0f, v.h, 0.0f },
           { v.w/2.0,  v.h, 0.0f }
        }
    );

    GlTexture font_map(LoadImage("/Users/stevenlovegrove/code/msdf-atlas-gen/fonts/zcool/站酷仓耳渔阳体-W02.ttf_map.png"));
    CheckGlDieOnError();

    pangolin::GlSlProgram prog;
    prog.AddShader( pangolin::GlSlAnnotatedShader, my_shader );
    prog.BindPangolinDefaultAttribLocationsAndLink();

    GlVertexArrayObject vao;
    vao.AddVertexAttrib(pangolin::DEFAULT_LOCATION_POSITION, vbo);
    vao.Unbind();

    DisplayBase().Activate();

    auto T_cm = ProjectionMatrixOrthographic(-0.5, v.w-0.5, -0.5, v.h-0.5, -1.0, 1.0);

    float dx = 0.0;

    while( !pangolin::ShouldQuit() )
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        CheckGlDieOnError();

        if(true) {
            prog.Bind();
            CheckGlDieOnError();

            prog.SetUniform("u_T_cm", T_cm);
            vao.Bind();
            glActiveTexture(GL_TEXTURE0);
            font_map.Bind();
            glDrawArrays(GL_TRIANGLE_STRIP, 0, vbo.num_elements);
            font_map.Unbind();
            prog.Unbind();
        }

//        exit(0);
        pangolin::FinishFrame();
    }
}

int main( int /*argc*/, char** /*argv*/ )
{
//    test();
    sample2();
    return 0;
}
