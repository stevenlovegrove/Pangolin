@start vertex
#version 150 core

in vec3 a_position;
in uint a_char_index;
out vec3 pos;
out uint index;

void main() {
    pos = a_position;
    index = a_char_index;
}

@start geometry
#version 150 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

#include "font_offset_table.glsl.h"

uniform mat4 u_T_cm;

in vec3 pos[];
in uint index[];
out vec2 v_pos;
out vec2 v_win;
flat out uint char_id;

uniform float u_scale;

void main() {
    char_id = index[0];
    vec4 font_offset;
    vec4 screen_offset;
    font_and_screen_offset(char_id, font_offset, screen_offset);

    float w = u_scale * screen_offset.z;
    float h = u_scale * screen_offset.w;

    // 0, 1
    // 2, 3

    vec2 corners[4] = vec2[](
        vec2(0.0,0.0), vec2(w,0.0),
        vec2(0.0,h), vec2(w,h)
    );

    for(uint i=0u; i < 4u;  ++i) {
        v_win = pos[0].xy + u_scale*screen_offset.xy + corners[i];
        v_pos = corners[i];
        gl_Position = u_T_cm * vec4(v_win, pos[0].z, 1.0);
        EmitVertex();
    }

    EndPrimitive();}

@start fragment
#version 150 core
#include "utils.glsl.h"
#include "sdf.glsl.h"
#include "font.glsl.h"

in vec2  v_pos;
flat in uint   char_id;
out vec4 FragColor;

uniform float u_scale;

void main() {
    vec2 pos = v_pos / u_scale;
    float sdf = sdf_font(int(char_id), pos);
    float opacity = clamp( sdf + 0.5, 0.0, 1.0);
    FragColor = vec4( vec3(0.0), opacity );
}
