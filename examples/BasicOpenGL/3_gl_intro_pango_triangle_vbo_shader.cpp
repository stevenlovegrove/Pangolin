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
uniform sampler2D u_matcap;
uniform float u_val;

const vec2 light_dir = vec2(-sqrt(0.5), -sqrt(0.5));
const vec3 light_dir3 = vec3(-sqrt(1.0/3.0));
const float M_PI = 3.1415926535897932384626433832795;

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

float sdf_line_segment(vec2 p, vec2 a, vec2 b) {
    vec2 ba = b - a;
    vec2 pa = p - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0., 1.);
    return length(pa - h * ba);
}

vec2 grad( ivec2 z )  // replace this anything that returns a random vector
{
    // 2D to 1D  (feel free to replace by some other)
    int n = z.x+z.y*11111;

    // Hugo Elias hash (feel free to replace by another one)
    n = (n<<13)^n;
    n = (n*(n*n*15731+789221)+1376312589)>>16;

    // Perlin style vectors
    n &= 7;
    vec2 gr = vec2(n&1,n>>1)*2.0-1.0;
    return ( n>=6 ) ? vec2(0.0,gr.x) :
           ( n>=4 ) ? vec2(gr.x,0.0) :
                              gr;
}

float noise( in vec2 p )
{
    ivec2 i = ivec2(floor( p ));
     vec2 f =       fract( p );

    vec2 u = f*f*(3.0-2.0*f); // feel free to replace by a quintic smoothstep instead

    return mix( mix( dot( grad( i+ivec2(0,0) ), f-vec2(0.0,0.0) ),
                     dot( grad( i+ivec2(1,0) ), f-vec2(1.0,0.0) ), u.x),
                mix( dot( grad( i+ivec2(0,1) ), f-vec2(0.0,1.0) ),
                     dot( grad( i+ivec2(1,1) ), f-vec2(1.0,1.0) ), u.x), u.y);
}

vec3 matcap(vec3 normal)
{
    vec2 t = (normal.xy + vec2(1.0,1.0)) / 2.0;
    return texture(u_matcap, t).xyz;
}

vec4 eg1() {
    float half_height = 20.0;
    float padding = 24.0;
    float rad = half_height * 0.5;
    vec2 p = vec2(v_pos.x, mod(v_pos.y, 2*(padding+half_height) ) );

    float sdf = sdf_rounded_rect(p, vec2(padding+100.0, padding+half_height), vec2(100, half_height), rad);
    vec2 dsdf = vec2(dFdx(sdf), dFdy(sdf));
    dsdf /= length(dsdf);

    return mix3(
            vec4(0.8,0.8,0.8,1.0),
            vec4(vec3(0.5, 0.5, 0.5) + dot(dsdf,light_dir) * vec3(0.5, 0.0, 0.0), 1.0),
            vec4(0.9,0.9,0.9,1.0),
            sdf);
}

vec4 eg2() {
    float half_height = 25.0;
    float padding = 15.0;
    float rad = half_height * 0.3;
    float width = 150.0;
    vec2 p = vec2(v_pos.x, mod(v_pos.y, 2*(padding+half_height) ) );

    float sdf = sdf_rounded_rect(p, vec2(padding+width, padding+half_height), vec2(width, half_height), rad);
    float h = 0.0;
    if(sdf < 0.0) {
        h = rad;
    }else if(sdf <= rad) {
        float x = sdf / rad;
        h = rad - rad * (1.0 - sqrt(1.0 - x*x));
    }else if(sdf <= rad+rad/2.0) {
        float x = -(sdf-rad/2.0) / rad;
        h = rad * (1.0 - sqrt(1.0 - x*x));
//    }else if(sdf <= rad+rad) {
//        float x = (sdf-3.0*rad/2.0) / rad;
//        h = rad * (1.0 - sqrt(1.0 - x*x));
    }else{
        h=rad;
    }

    h += 0.2*noise(p*8.0+vec2(20.4));

//    }else if(sdf < rad) {
//        float x = sdf / rad;
//        h = rad * (1.0 - sqrt(1.0 - x*x));
//    }else if(sdf < rad+2) {
//        h = 0.0;
//    }else{
//        float x = -(sdf-(rad+2.0)) / rad;
//        h = rad * sqrt(1.0 - x*x);
//    }

    vec3 n = vec3(dFdx(h), dFdy(h), 1.0);
    vec3 norm = n / length(n);

//    return vec4(vec3(0.5, 0.5, 0.5) + dot(norm,light_dir3) * vec3(0.5, 0.0, 0.0), 1.0);
    return vec4(matcap(norm), 1.0);
}

vec2 wave(float x, float center, float rad)
{
    float phase = clamp( (x - center) / rad, -1.0, 1.0);
    float y = (1+cos(phase*M_PI))/2.0;
    float dy_dx = -0.5*M_PI*sin(phase*M_PI)/rad;
    return vec2(y, dy_dx);
}

vec4 eg3() {
    float half_height = 25.0;
    float padding = 15.0;
    float rad = 50.0;
    float height = 40.0;
    float width = 400.0;
    float val_pix = u_val*width;
    float circ_rad = 5;

    vec2 p = vec2(v_pos.x, mod(v_pos.y, 2*(padding+half_height) ) );
    vec2 xy = p - vec2(padding);
    vec2 y_dy = height * wave(xy.x, val_pix, rad);

    // distance to wave
    float dist_wave = abs(xy.y - y_dy.x) / sqrt(1.0 + y_dy.y*y_dy.y);
    if(xy.x < 0.0 || xy.x > width) dist_wave = 1e6;

    // distance to start circle
    float dist_c1 = length(xy - vec2(0.0,height*wave(0.0, val_pix, rad).x )) - circ_rad;

    // distance to end circle
    float dist_c2 = length(xy - vec2(width,height*wave(width, val_pix, rad).x )) - circ_rad;

    float de = min(min(dist_wave, dist_c1), dist_c2);

    vec3 v = mix( vec3(0.9), vec3(1.0,0.6,0.2), 1.0-smoothstep( 3.0, 4.0, de ) );

    return vec4(v,1.0);
}

void main() {
    FragColor = eg3();
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

    int x,y;

};

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

//    GlTexture font_map(LoadImage("/Users/stevenlovegrove/code/msdf-atlas-gen/fonts/zcool/站酷仓耳渔阳体-W02.ttf_map.png"));
//    CheckGlDieOnError();

    GlTexture matcap;
    matcap.LoadFromFile("/Users/stevenlovegrove/Downloads/matcap1.png");
//    matcap.LoadFromFile("/Users/stevenlovegrove/Downloads/matcap3.jpg");
//    matcap.LoadFromFile("/Users/stevenlovegrove/Downloads/matcap_normal.jpg");


    pangolin::GlSlProgram prog;
    prog.AddShader( pangolin::GlSlAnnotatedShader, my_shader );
    prog.BindPangolinDefaultAttribLocationsAndLink();

    GlVertexArrayObject vao;
    vao.AddVertexAttrib(pangolin::DEFAULT_LOCATION_POSITION, vbo);
    vao.Unbind();

    DisplayBase().Activate();

    auto T_cm = ProjectionMatrixOrthographic(-0.5, v.w-0.5, -0.5, v.h-0.5, -1.0, 1.0);

    float dx = 0.0;
    float time = 0.0;

    while( !pangolin::ShouldQuit() )
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(true) {
            prog.Bind();

            prog.SetUniform("u_T_cm", T_cm);
            prog.SetUniform("u_val", std::clamp((handler.x - 15.0f)/400.0f, 0.0f, 1.0f ) );
//            prog.SetUniform("u_val", time);
//            time = fmod(time+0.001, 1.0);

            vao.Bind();
            glActiveTexture(GL_TEXTURE0);
            matcap.Bind();
            glDrawArrays(GL_TRIANGLE_STRIP, 0, vbo.num_elements);
            matcap.Unbind();
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
