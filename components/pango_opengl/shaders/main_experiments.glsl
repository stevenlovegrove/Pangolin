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
#include "utils.glsl.h"
#include "colormaps.glsl.h"
#include "sdf.glsl.h"
#include "font.glsl.h"

in vec2 v_pos;
out vec4 FragColor;
uniform float u_val;
uniform vec2 u_mouse_pos;
uniform int u_char_id;
uniform usampler2D u_text;

const vec2 light_dir = vec2(-sqrt(0.5), -sqrt(0.5));
const float M_PI = 3.1415926535897932384626433832795;

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

vec4 slider(bool button)
{
    // dimensions
    float padding = 15.0;
    float half_width = 200.0;
    float half_height = 25.0;

    // Style params
    float slider_outline_border = 2;
    float boss_border = 1;
    float boss_radius = 25;

    vec3 color_panel = vec3(0.8);
    vec3 color_boss_base = color_panel;
    vec3 color_boss_diff = vec3(0.2, 0.15, 0.20);
    vec3 color_slider = vec3(0.9, 0.7, 0.7);
    vec3 color_slider_outline = color_slider - vec3(0.1);

//    // Style params
//    float half_height = 35.0;
//    float boss_border = 3;
//    float boss_radius = 35;
//    float slider_outline_border = 4;

//    vec3 color_panel = vec3(0.95);
//    vec3 color_boss_base = vec3(0.0);
//    vec3 color_boss_diff = vec3(0, 0, 0);
//    vec3 color_slider = vec3(0.7, 0.7, 0.7);
//    vec3 color_slider_outline = vec3(0.95);

    // maths
    float val_pix = u_val*2.0*half_width;
    float frac_y = mod(v_pos.y, 2*(padding+half_height) );
    vec2 p = vec2(v_pos.x, frac_y );
    float pos_along_slider = clamp((p.x-padding) / val_pix, 0.0, 1.0);

    float panel_sdf = sdf_rounded_rect(v_pos, vec2(half_width+padding, 15*half_height), vec2(half_width+2*padding, 15*(half_height+padding)), boss_radius);
    float box_sdf   = sdf_rounded_rect(p, vec2(padding+half_width, padding+half_height), vec2(half_width, half_height), boss_radius);
    vec2  box_grad = normalize(vec2(dFdx(box_sdf), dFdy(box_sdf)) + vec2(0.0001, 0.0001)); // (eps to avoid bad normalization)
    float slide_sdf = sdf_rounded_rect(p, vec2(padding+val_pix/2.0, padding+half_height), vec2(val_pix/2.0, half_height), boss_radius);
    vec2  slide_grad = normalize(vec2(dFdx(slide_sdf), dFdy(slide_sdf)));

    bool pressed = button && ((u_char_id % 2) == 0);

    // Panel
    vec4 v = color_sdf(panel_sdf, color_panel);

    if(pressed) box_grad*= -1;
    vec3 color_boss = color_boss_base + dot(box_grad,light_dir) * color_boss_diff;

    // Add indented (embossed) area for slider / button
    v = composite( color_sdf(box_sdf, color_panel, color_boss, boss_border ), v);

    if(!button) {
        // Add Slider with outline
        v = composite( color_sdf(slide_sdf+boss_border, color_slider, color_slider_outline, slider_outline_border ), v);
    }

    // Add text
    vec2 font_pos   = 1.5*p - vec2(50.0, padding + half_height + 10 );
    int font_i = int(font_pos.x / 20.0);
    font_pos.x = font_pos.x >= 0.0 ? mod(font_pos.x, 20.0) : -1;

    if(0.0 <= font_pos.x && 0 <= font_i && font_i < 10) {
        uint char_id = texelFetch(u_text, ivec2(font_i, 0), 0).r;
        if(0u < char_id) {
            float font_opacity = font_color(int(char_id), font_pos);
            v = composite(vec4(vec3(0.0),font_opacity), v);
        }
    }

    return v;
}

vec4 font_render() {
    const float padding = 15.0;
    float opacity = font_color(u_char_id, 1.5*v_pos - vec2(10,padding+25+10));
    return vec4(vec3(0.0),opacity);
}

void main() {
    FragColor = slider(false);
//    FragColor = slider_wave();
//    FragColor = font_render();
}
