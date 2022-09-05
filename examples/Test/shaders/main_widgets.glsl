@start vertex
#version 150 core

in vec4 a_position;
out vec4 pos;

void main() {
    pos = a_position;
}

@start geometry
#version 150 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 u_T_cm;

in vec4 pos[];
out vec2 v_pos;
out vec2 v_win;
flat out float val;
flat out uint  widget_type;

uniform float u_width;
uniform float u_height;

void output_widget()
{
    val = pos[0].z;
    widget_type = uint(pos[0].w);
    vec2 corners[4] = vec2[](
        vec2(0.0,0.0), vec2(u_width,0.0),
        vec2(0.0,u_height), vec2(u_width,u_height)
    );

    for(uint i=0u; i < 4u;  ++i) {
        v_win = pos[0].xy + corners[i];
        v_pos = corners[i];
        gl_Position = u_T_cm * vec4(v_win, 0.0, 1.0);
        EmitVertex();
    }

    EndPrimitive();
}

void main() {
    output_widget();
}

@start fragment
#version 150 core
#include "utils.glsl"
#include "colormaps.glsl"
#include "sdf.glsl"

in vec2 v_pos;
in vec2 v_win;
flat in float val;
flat in uint  widget_type;
out vec4 FragColor;

// Dimensions
uniform float u_width;
uniform float u_height;
uniform float u_padding;
uniform int u_num_widgets;

// Style params
uniform float slider_outline_border;
uniform float boss_border;
uniform float boss_radius_factor;

uniform vec3 color_panel;
uniform vec3 color_boss_base;
uniform vec3 color_boss_diff;
uniform vec3 color_slider;
uniform vec3 color_slider_outline;

const vec2 light_dir = vec2(sqrt(0.5), sqrt(0.5));
const float M_PI = 3.1415926535897932384626433832795;

vec4 widget()
{
    // widget_type:
    // 0: Label
    // 1: Textbox
    // 2: button
    // 3: slider

    bool is_button   = widget_type==2u;
    bool is_slider   = widget_type==3u;
    bool is_embossed = !is_button || (val > 0.5);

    float boss_border2 = is_button ? boss_border + 2.0 : boss_border;

    // dimensions
    float padding = u_padding;
    float half_width = u_width/2.0 - padding;
    float half_height = u_height/2.0 - padding;
    float boss_radius = half_height * boss_radius_factor;

    // maths
    float val_pix = val*2.0*half_width;
    float pos_along_slider = clamp((v_pos.x-padding) / val_pix, 0.0, 1.0);

    int panel_n = u_num_widgets;
    float panel_sdf = sdf_rounded_rect(v_win, vec2(half_width+padding, panel_n*(half_height+padding)), vec2(half_width+padding, panel_n*(half_height+padding)), boss_radius);
    float box_sdf   = sdf_rounded_rect(v_pos, vec2(padding+half_width, padding+half_height), vec2(half_width, half_height), boss_radius);
    vec2  box_grad = normalize(vec2(dFdx(box_sdf), dFdy(box_sdf)) + vec2(0.0001, 0.0001)); // (eps to avoid bad normalization)
    float slide_sdf = sdf_rounded_rect(v_pos, vec2(padding+val_pix/2.0, padding+half_height), vec2(val_pix/2.0, half_height), boss_radius);
    vec2  slide_grad = normalize(vec2(dFdx(slide_sdf), dFdy(slide_sdf)));


    // Panel
    vec4 v = color_sdf(panel_sdf, color_panel);

    if(is_button || is_slider) {
        if(is_embossed) box_grad*= -1;
        vec3 color_boss = color_boss_base + dot(box_grad,light_dir) * color_boss_diff;

        // Add indented (embossed) area for slider / button
        v = composite( color_sdf(box_sdf, color_panel, color_boss, boss_border2 ), v);
    }

    if(is_slider) {
        // Add Slider with outline
        v = composite( color_sdf(slide_sdf+boss_border2, color_slider, color_slider_outline, slider_outline_border ), v);
    }

    return v;
}

void main() {
    FragColor = widget();
}
