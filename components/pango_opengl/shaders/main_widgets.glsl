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
flat out float divisions;
flat out float val;
flat out int  selected_index;
flat out int  widget_index;
flat out uint  widget_type;


void output_widget()
{
    divisions = floor(pos[0].z);
    val = 2.0*(pos[0].z - divisions);

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
#include "utils.glsl.h"
#include "colormaps.glsl.h"
#include "sdf.glsl.h"

in vec2 v_pos;
in vec2 v_win;
flat in float  divisions;
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

float padding = u_padding;
float half_width = u_width/2.0 - padding;
float half_height = u_height/2.0 - padding;
float boss_radius = half_height * boss_radius_factor;

vec4 widget_panel(vec2 v_win)
{
    vec3 color_panel_border = vec3(0.9);
    float border_panel = 3;

    float panel_sdf = sdf_rounded_rect(v_win, vec2(half_width+padding, u_num_widgets*(half_height+padding)), vec2(half_width+padding, u_num_widgets*(half_height+padding)), boss_radius);
    return color_sdf(panel_sdf, color_panel, color_panel_border, border_panel);
}

vec4 widget_seperator()
{
    float line_sdf  = sdf_line_segment(v_pos, vec2(2*padding,2.2*half_height), vec2(u_width-2*padding, 2.2*half_height));
    vec4 v = widget_panel(v_win);
    v = composite( color_sdf(line_sdf-1, vec3(0.7)), v);
    return v;
}

vec4 widget()
{
    // widget_type:
    // 0: Label
    // 1: Textbox
    // 2: button
    // 3: checkbox
    // 4: slider
    // 5: seperator

    bool is_textbox   = widget_type==1u;
    bool is_button    = widget_type==2u || widget_type==3u;
    bool is_slider    = widget_type==4u;
    bool is_seperator = widget_type==5u;

    bool is_toggled  = val > 0.5;
    bool is_active_widget = selected_index == widget_index;

    if(is_seperator) return widget_seperator();

    // dimensions
    float padding = u_padding;
    float half_width = u_width/2.0 - padding;
    float half_height = u_height/2.0 - padding;
    float boss_radius = half_height * boss_radius_factor;

    // maths
    float val_pix = val*2.0*half_width;

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

    float box_sdf   = sdf_rounded_rect(v_pos, vec2(padding+half_width, padding+half_height), vec2(half_width, half_height), boss_radius);
    vec2  box_grad = normalize(vec2(dFdx(box_sdf), dFdy(box_sdf)) + vec2(0.0001, 0.0001)); // (eps to avoid bad normalization)
    float slide_sdf = sdf_rounded_rect(v_pos, vec2(slider_center, padding+half_height), vec2(slider_width, inside_height), inside_radius);

    // Panel
    vec4 v = widget_panel(v_win);

    if(is_button || is_slider || is_textbox) {
        vec3 color_boss = color_boss_base - /*dot(box_grad,light_dir) **/ color_boss_diff;
        float border = boss_border;
        if(is_active_widget) {
            color_boss.x += .3;
            border += 1;
        }

        if(is_textbox) {
            v = composite( color_sdf(box_sdf, vec3(0.9), color_boss, border ), v);
        }else{
            // Add indented (embossed) area for slider / button
            v = composite( color_sdf(box_sdf, color_panel, color_boss, border ), v);
        }
    }

    if(is_button || is_slider) {
        // Add Slider with outline
        v = composite( color_sdf(slide_sdf+boss_border, color_inside, color_inside_outline, border_inside ), v);
    }

    // WIP: markings
    if(is_slider) {
        float val_here = divisions * (v_pos.x-padding) / (2.0*(u_width/2.0 - padding));
        float val_closest = round(val_here);
        float msdf = mod(abs(val_closest-val_here),1.0);
        float alpha = 1.0 - smoothstep(0.0, 1.0, box_sdf);
        float dist_scale = half_width / divisions;
        vec4 c = color_sdf(dist_scale*msdf, color_inside_outline);
        c.a *= alpha;
        v = composite( c, v);
    }

    return v;
}

void main() {
    FragColor = widget();
}
