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
uniform float u_width;
uniform float u_height;
uniform int u_selected_index;

in vec4 pos[];
out vec2 v_pos;
out vec2 v_win;
flat out float val;
flat out int  selected_index;
flat out int  widget_index;
flat out uint  widget_type;


void output_widget()
{
    val = pos[0].z;
    widget_type = uint(pos[0].w);
    selected_index = u_selected_index;
    widget_index = int(pos[0].y);

    vec2 corners[4] = vec2[](
        vec2(0.0,0.0), vec2(u_width,0.0),
        vec2(0.0,u_height), vec2(u_width,u_height)
    );

    for(uint i=0u; i < 4u;  ++i) {
        v_win = vec2(0.0, u_height*pos[0].y) + corners[i];
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
flat in int  selected_index;
flat in int  widget_index;
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

const vec2 light_dir = vec2(-sqrt(0.5), -sqrt(0.5));
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
    bool is_toggled  = val > 0.5;
    bool is_active_widget = selected_index == widget_index;

    // dimensions
    float padding = u_padding;
    float half_width = u_width/2.0 - padding;
    float half_height = u_height/2.0 - padding;
    float boss_radius = half_height * boss_radius_factor;

    // maths
    float val_pix = val*2.0*half_width;
    float pos_along_slider = clamp((v_pos.x-padding) / val_pix, 0.0, 1.0);

    float slider_center = padding+val_pix/2.0;
    float slider_width  = val_pix/2.0;
    vec3 color_inside = color_slider;
    vec3 color_inside_outline = color_slider_outline;
    float border_inside = slider_outline_border;
    float inside_height = half_height;
    float inside_radius = half_height;

    if(is_button) {
        slider_center = 2*half_width-half_height;
        slider_width = half_height+padding;
        color_inside_outline = color_slider_outline;
        color_inside = is_toggled ? vec3(0.9,0.8,0.8): vec3(0.8);
        border_inside = 2;
        slider_width -= 4;
        inside_height -= 4;
        inside_radius -= 4;
    }

    int panel_n = u_num_widgets;
    float panel_sdf = sdf_rounded_rect(v_win, vec2(half_width+padding, panel_n*(half_height+padding)), vec2(half_width+padding, panel_n*(half_height+padding)), boss_radius);
    float box_sdf   = sdf_rounded_rect(v_pos, vec2(padding+half_width, padding+half_height), vec2(half_width, half_height), boss_radius);
    vec2  box_grad = normalize(vec2(dFdx(box_sdf), dFdy(box_sdf)) + vec2(0.0001, 0.0001)); // (eps to avoid bad normalization)
    float slide_sdf = sdf_rounded_rect(v_pos, vec2(slider_center, padding+half_height), vec2(slider_width, inside_height), inside_radius);

    // Panel
    vec4 v = color_sdf(panel_sdf, color_panel);

    if(is_button || is_slider) {
        vec3 color_boss = color_boss_base - /*dot(box_grad,light_dir) **/ color_boss_diff;
        float border = boss_border;
        if(is_active_widget) {
            color_boss.x += .3;
            border += 1;
        }

        // Add indented (embossed) area for slider / button
        v = composite( color_sdf(box_sdf, color_panel, color_boss, border ), v);
    }

//    if(is_slider)
    {
        // Add Slider with outline
        v = composite( color_sdf(slide_sdf+boss_border, color_inside, color_inside_outline, border_inside ), v);
    }

    return v;
}

void main() {
    FragColor = widget();
}
