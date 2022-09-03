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

uniform mat4 u_T_cm;

in vec3 pos[];
in uint index[];
out vec2 v_pos;
out vec2 v_win;
flat out uint char_id;

const bool flip_y = true;

uniform float u_scale;

void main() {
    float w = u_scale * 32.0;
    float h = u_scale * 32.0;

    float expand = 0.0;

//    vec2 corners[4] = vec2[](
//        vec2(0.0,0.0), vec2(w,0.0),
//        vec2(0.0,h), vec2(w,h)
//    );
    vec2 corners[4] = vec2[](
        vec2(-expand,-expand), vec2(w+expand,-expand),
        vec2(-expand,h+expand), vec2(w+expand,h+expand)
    );

    char_id = index[0];

    for(uint i=0u; i < 4u;  ++i) {
        v_win = pos[0].xy + corners[i];
        v_pos = corners[i];
        if(flip_y) v_pos.y = h - v_pos.y;
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
    vec2 pos = v_pos / u_scale - vec2(0,8);
    float sdf = sdf_font(int(char_id), pos);
    float opacity = clamp( sdf + 0.5, 0.0, 1.0);
    FragColor = vec4( vec3(0.0), opacity );
}
