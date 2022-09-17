#include <pangolin/display/display.h>
#include <pangolin/display/default_font.h>
#include <pangolin/var/var.h>
#include <pangolin/display/widgets.h>

#include <pangolin/gl/gldraw.h>
#include <pangolin/gl/glvbo.h>
#include <pangolin/gl/glsl.h>
#include <pangolin/gl/glfont.h>

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

const bool is_msdf = false;
const bool is_alpha = false;

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
  if(is_alpha) {
      gl_FragColor = vec4(vec3(sample.a), 1.0);
  }else{
      vec3 msd = sample.xyz;
      float sd = is_msdf ? median(msd.r, msd.g, msd.b) : msd.r;
      float screenPxDistance = screenPxRange()*(sd - 0.5);
      float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
      gl_FragColor = mix(u_color_bg, u_color_fg, opacity);
  }

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
uniform sampler2D u_font_atlas;
uniform sampler2D u_font_offsets;
uniform float u_val;
uniform int u_char_id;
uniform vec2 u_mouse_pos;

const vec2 light_dir = vec2(-sqrt(0.5), -sqrt(0.5));
const vec3 light_dir3 = vec3(-sqrt(1.0/3.0));
const float M_PI = 3.1415926535897932384626433832795;

// From https://www.shadertoy.com/view/WlfXRN
vec3 plasma(float t) {
    const vec3 c0 = vec3(0.05873234392399702, 0.02333670892565664, 0.5433401826748754);
    const vec3 c1 = vec3(2.176514634195958, 0.2383834171260182, 0.7539604599784036);
    const vec3 c2 = vec3(-2.689460476458034, -7.455851135738909, 3.110799939717086);
    const vec3 c3 = vec3(6.130348345893603, 42.3461881477227, -28.51885465332158);
    const vec3 c4 = vec3(-11.10743619062271, -82.66631109428045, 60.13984767418263);
    const vec3 c5 = vec3(10.02306557647065, 71.41361770095349, -54.07218655560067);
    const vec3 c6 = vec3(-3.658713842777788, -22.93153465461149, 18.19190778539828);
    return c0+t*(c1+t*(c2+t*(c3+t*(c4+t*(c5+t*c6)))));
}
vec3 viridis(float t) {
    const vec3 c0 = vec3(0.2777273272234177, 0.005407344544966578, 0.3340998053353061);
    const vec3 c1 = vec3(0.1050930431085774, 1.404613529898575, 1.384590162594685);
    const vec3 c2 = vec3(-0.3308618287255563, 0.214847559468213, 0.09509516302823659);
    const vec3 c3 = vec3(-4.634230498983486, -5.799100973351585, -19.33244095627987);
    const vec3 c4 = vec3(6.228269936347081, 14.17993336680509, 56.69055260068105);
    const vec3 c5 = vec3(4.776384997670288, -13.74514537774601, -65.35303263337234);
    const vec3 c6 = vec3(-5.435455855934631, 4.645852612178535, 26.3124352495832);
    return c0+t*(c1+t*(c2+t*(c3+t*(c4+t*(c5+t*c6)))));
}
vec3 magma(float t) {
    const vec3 c0 = vec3(-0.002136485053939582, -0.000749655052795221, -0.005386127855323933);
    const vec3 c1 = vec3(0.2516605407371642, 0.6775232436837668, 2.494026599312351);
    const vec3 c2 = vec3(8.353717279216625, -3.577719514958484, 0.3144679030132573);
    const vec3 c3 = vec3(-27.66873308576866, 14.26473078096533, -13.64921318813922);
    const vec3 c4 = vec3(52.17613981234068, -27.94360607168351, 12.94416944238394);
    const vec3 c5 = vec3(-50.76852536473588, 29.04658282127291, 4.23415299384598);
    const vec3 c6 = vec3(18.65570506591883, -11.48977351997711, -5.601961508734096);
    return c0+t*(c1+t*(c2+t*(c3+t*(c4+t*(c5+t*c6)))));
}

vec3 inferno(float t) {
    const vec3 c0 = vec3(0.0002189403691192265, 0.001651004631001012, -0.01948089843709184);
    const vec3 c1 = vec3(0.1065134194856116, 0.5639564367884091, 3.932712388889277);
    const vec3 c2 = vec3(11.60249308247187, -3.972853965665698, -15.9423941062914);
    const vec3 c3 = vec3(-41.70399613139459, 17.43639888205313, 44.35414519872813);
    const vec3 c4 = vec3(77.162935699427, -33.40235894210092, -81.80730925738993);
    const vec3 c5 = vec3(-71.31942824499214, 32.62606426397723, 73.20951985803202);
    const vec3 c6 = vec3(25.13112622477341, -12.24266895238567, -23.07032500287172);
    return c0+t*(c1+t*(c2+t*(c3+t*(c4+t*(c5+t*c6)))));
}

// https://www.shadertoy.com/view/4dXXDX
// if edge0 < x <= edge1, return 1.0, otherwise return 0
float segment(float edge0, float edge1, float x)
{
    return step(edge0,x) * (1.0-step(edge1,x));
}
vec3 gray(float t)
{
    return vec3(t);
}
vec3 hot(float t)
{
    return vec3(smoothstep(0.00,0.33,t),
                smoothstep(0.33,0.66,t),
                smoothstep(0.66,1.00,t));
}
vec3 cool(float t)
{
    return mix( vec3(0.0,1.0,1.0), vec3(1.0,0.0,1.0), t);
}
vec3 autumn(float t)
{
    return mix( vec3(1.0,0.0,0.0), vec3(1.0,1.0,0.0), t);
}
vec3 winter(float t)
{
    return mix( vec3(0.0,0.0,1.0), vec3(0.0,1.0,0.5), sqrt(t));
}
vec3 spring(float t)
{
    return mix( vec3(1.0,0.0,1.0), vec3(1.0,1.0,0.0), t);
}
vec3 summer(float t)
{
    return mix( vec3(0.0,0.5,0.4), vec3(1.0,1.0,0.4), t);
}
vec3 ice(float t)
{
   return vec3(t, t, 1.0);
}
vec3 fire(float t)
{
    return mix( mix(vec3(1,1,1), vec3(1,1,0), t),
                mix(vec3(1,1,0), vec3(1,0,0), t*t), t);
}
vec3 ice_and_fire(float t)
{
    return segment(0.0,0.5,t) * ice(2.0*(t-0.0)) +
           segment(0.5,1.0,t) * fire(2.0*(t-0.5));
}
vec3 reds(float t)
{
    return mix(vec3(1,1,1), vec3(1,0,0), t);
}
vec3 greens(float t)
{
    return mix(vec3(1,1,1), vec3(0,1,0), t);
}
vec3 blues(float t)
{
    return mix(vec3(1,1,1), vec3(0,0,1), t);
}
// By Morgan McGuire
vec3 wheel(float t)
{
    return clamp(abs(fract(t + vec3(1.0, 2.0 / 3.0, 1.0 / 3.0)) * 6.0 - 3.0) -1.0, 0.0, 1.0);
}
// By Morgan McGuire
vec3 stripes(float t)
{
    return vec3(mod(floor(t * 64.0), 2.0) * 0.2 + 0.8);
}

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

vec4 slider_very_flat() {
    float half_width = 200;
    float half_height = 30.0;
    float padding = 24.0;
    float rad = half_height * 0.8;
    vec2 p = vec2(v_pos.x, mod(v_pos.y, 2*(padding+half_height) ) );

    float sdf = sdf_rounded_rect(p, vec2(padding+half_width, padding+half_height), vec2(half_width, half_height), rad);
    vec2 dsdf = vec2(dFdx(sdf), dFdy(sdf));
    dsdf /= length(dsdf);

    return mix3(
            vec4(0.85,0.85,0.85,1.0),
            vec4(vec3(0.5, 0.5, 0.5) + dot(dsdf,light_dir) * vec3(0.5, 0.0, 0.0), 1.0),
            vec4(0.9,0.9,0.9,1.0),
            sdf/2.0);
}

vec2 wave(float x, float center, float rad)
{
    float phase = clamp( (x - center) / rad, -1.0, 1.0);
    float y = (1+cos(phase*M_PI))/2.0;
    float dy_dx = -0.5*M_PI*sin(phase*M_PI)/rad;
    return vec2(y, dy_dx);
}

vec4 slider_wave() {
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


float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

float screenPxRange(vec2 tex_coord) {
    const float pxRange = 2.0;
    vec2 unitRange = vec2(pxRange)/vec2(textureSize(u_font_atlas, 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(tex_coord);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

vec2 clamp2(vec2 v, vec2 low, vec2 high)
{
    return vec2(
        clamp(v.x, low.x, high.x),
        clamp(v.y, low.y, high.y)
    );
}

float font_color(vec4 atlas_offset, vec2 pos )
{
    float atlas_dim = textureSize(u_font_atlas, 0).x;

    vec2 p = vec2(1,-1) * clamp2(
        pos/atlas_dim,
        vec2(0.0, 0.0),
        vec2(atlas_offset.z, -atlas_offset.w)
    );

    vec2 uv = atlas_offset.xy + p;
    vec2 tex = vec2(uv.x, uv.y);
    vec3 msd = texture(u_font_atlas, tex).xyz;
    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = screenPxRange(tex)*(sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    return opacity;
}

vec4 composite(vec4 top_layer, vec4 bottom_layer)
{
    float alpha = top_layer.a;
    return alpha * top_layer + (1.0 - alpha) * bottom_layer;
}

vec4 slider(bool button)
{
    float half_height = 30.0;
    float border = 2;
    float half_height_slider = half_height - border;

    float padding = 15.0;
    float half_width = 200.0;
    float half_width_slider = 195.0;
    float val_pix = u_val*2.0*half_width;
    float frac_y = mod(v_pos.y, 2*(padding+half_height) );
    vec2 p = vec2(v_pos.x, frac_y );
    float pos_along_slider = clamp((p.x-padding) / val_pix, 0.0, 1.0);

//    vec2 light_dir = normalize(u_mouse_pos - v_pos);

    float dist_panel = sdf_rounded_rect(v_pos, vec2(half_width+padding, 15*half_height), vec2(half_width+2*padding, 15*half_height), half_height);
    float panel_opac = smoothstep(0.0, 1.0, -dist_panel);

    float dist_box   = sdf_rounded_rect(p, vec2(padding+half_width, padding+half_height), vec2(half_width, half_height), half_height);
    float dist_slide = sdf_rounded_rect(p, vec2(padding+val_pix/2.0, padding+half_height), vec2(val_pix/2.0-border, half_height_slider), half_height_slider);

    vec2 dsdf = normalize(vec2(dFdx(dist_box), dFdy(dist_box)));
    vec2 dsdf_slide = normalize(vec2(dFdx(dist_slide), dFdy(dist_slide)));
    if(u_char_id % 2 == 0) dsdf_slide*= -1;

    float a = smoothstep( -border, 0.0, dist_box );
    float b = 1.0 - smoothstep( 2.0, 4.0, dist_box );

    float d = smoothstep( -5, 0.0, dist_slide );
    float c = 1.0 - smoothstep( 0.0, 2.0, dist_slide );

    vec3 color_panel = vec3(0.8);
    vec3 color_boss = color_panel + dot(dsdf,light_dir) * vec3(0.2, 0.15, 0.20);
    vec4 color_bg = mix( vec4(color_panel,0.0), vec4(color_boss,1.0), a*b );

    vec3 color_button;
    vec3 color_edge;

    if(button) {
        // button-y style
        color_button = vec3(0.85, 0.85, 0.85);
        color_edge = color_panel - dot(dsdf_slide,light_dir) * vec3(0.2, 0.15, 0.20);
    }else{
        color_button = vec3(0.8) + 0.2*spring(pos_along_slider);
        color_button = vec3(0.9, 0.7, 0.7);
        color_edge = color_button - vec3(0.1);
    }

    vec3 color_fg = mix( color_button, color_edge, d );
    vec4 v = mix( color_bg, vec4(color_fg,1.0), c);

    v = composite(v, vec4(vec3(0.8),panel_opac));

    return v; //vec4(v,0.1);
}

vec4 font_render() {
    const float padding = 15.0;
    const float font_height = 32.0;
    vec4 font_offset = texelFetch(u_font_offsets, ivec2(u_char_id, 0), 0);
    vec2 screen_offset = texelFetch(u_font_offsets, ivec2(u_char_id, 1), 0).xy;
    vec3 v = vec3(font_color(font_offset, v_pos/10.0 - vec2(padding) - vec2(font_height/2.0) -screen_offset*font_height));

    return vec4(v,1.0);
}

void main() {
    FragColor = slider(false);
//    FragColor = slider_very_flat();
//    FragColor = slider_wave();
//    FragColor = font_render();
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//    pangolin::GlFont font("/Users/stevenlovegrove/code/msdf-atlas-gen/fonts/AnonymousPro.ttf_map.png", "/Users/stevenlovegrove/code/msdf-atlas-gen/fonts/AnonymousPro.ttf_map.json");
    pangolin::GlFont font("/Users/stevenlovegrove/code/Pangolin/components/pango_opengl/src/fonts/AnonymousPro.ttf", 32);
//    pangolin::GlFont font("/Users/stevenlovegrove/code/msdf-atlas-gen/fonts/anon_min/AnonymousPro.ttf_map.png", "/Users/stevenlovegrove/code/msdf-atlas-gen/fonts/anon_min/AnonymousPro.ttf_map.json");

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
            prog_text.SetUniform("u_offset", 10.0f /*+ dx*/, 10.0f );
            font.Text("Testing, 1, 2, 3...").DrawGlSl();
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

    while( !pangolin::ShouldQuit() )
    {
        glClearColor(0.7, 0.7, 0.7, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(true) {
            prog.Bind();

            prog.SetUniform("u_T_cm", T_cm);
            prog.SetUniform("u_val", std::clamp((handler.x - 15.0f)/400.0f, 0.0f, 1.0f ) );
            prog.SetUniform("u_font_atlas", 1);
            prog.SetUniform("u_font_offsets", 2);
            prog.SetUniform("u_mouse_pos", (float)handler.x, (float)handler.y);

            auto& co = font.chardata[handler.last_codepoint];
            prog.SetUniform("u_char_id", (int)co.AtlasIndex());

            vao.Bind();
            glActiveTexture(GL_TEXTURE0);
            matcap.Bind();
            glActiveTexture(GL_TEXTURE1);
            font.mTex.Bind();
            glActiveTexture(GL_TEXTURE2);
            font_offsets.Bind();
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
//    MainSliderExperiments();
    MainRenderTextWithNewAtlas();
    return 0;
}
