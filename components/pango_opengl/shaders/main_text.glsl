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
uniform sampler2D u_font_atlas;

uniform mat4 u_T_cm;

in vec3 pos[];
in uint index[];
out vec2 v_uv;
out vec2 v_win;

uniform float u_scale;

void main() {
    uint char_id = index[0];

    vec4 font_offset, screen_offset;
    font_and_screen_offset(char_id, font_offset, screen_offset);

    float w = u_scale * screen_offset.z;
    float h = u_scale * screen_offset.w;

    vec2 corners[4] = vec2[](
        vec2(0.0,0.0), vec2(w,0.0),
        vec2(0.0,h), vec2(w,h)
    );

    vec2 pos_scale = 1.0 / (u_scale * textureSize(u_font_atlas, 0) );

    for(uint i=0u; i < 4u;  ++i) {
        vec2 p = corners[i] * pos_scale ;
        v_uv = font_offset.xy + p;

        v_win = pos[0].xy + u_scale*screen_offset.xy + corners[i];

        gl_Position = u_T_cm * vec4(v_win, pos[0].z, 1.0);
        EmitVertex();
    }

    EndPrimitive();}

@start fragment
#version 150 core
#include "utils.glsl.h"
#include "sdf.glsl.h"

uniform sampler2D u_font_atlas;

in vec2  v_uv;
out vec4 FragColor;

uniform float u_scale;

float screenPxRange(vec2 tex_coord) {
    const float pxRange = 2.0;
    vec2 unitRange = vec2(pxRange)/vec2(textureSize(u_font_atlas, 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(tex_coord);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

void main() {
    vec3 msd = texture(u_font_atlas, v_uv).xyz;
    float sd = median(msd.r, msd.g, msd.b);
    float sdf = screenPxRange(v_uv)*(sd - 0.5);
    float opacity = clamp( sdf + 0.5, 0.0, 1.0);
    FragColor = vec4( vec3(0.0), opacity );
}
