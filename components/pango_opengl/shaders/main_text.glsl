@start vertex
#version 330

layout(location = 0) in vec3 a_position;
layout(location = 1) in uint a_char_index;
out vec3 pos_fontpix;
out uint index;

void main() {
    pos_fontpix = a_position;
    index = a_char_index;
}

@start geometry
#version 330
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

#include "font_offset_table.glsl.h"
uniform sampler2D u_font_atlas;

uniform mat4 u_clip_from_fontpix;

in vec3 pos_fontpix[];
in uint index[];
out vec2 v_uv;

void main() {
    uint char_id = index[0];
    vec4 font_offset_uv, screen_offset_fontpix;
    font_and_screen_offset(char_id, font_offset_uv, screen_offset_fontpix);

    vec2 display_offset_fontpix = screen_offset_fontpix.xy;
    vec2 dim_fontpix = screen_offset_fontpix.zw;

    vec2 corners_fontpix[4] = vec2[](
        vec2(0.0,0.0), vec2(dim_fontpix.x,0.0),
        vec2(0.0,dim_fontpix.y), dim_fontpix
    );

    for(uint i=0u; i < 4u;  ++i) {
        vec2 corner_fontpix = pos_fontpix[0].xy + display_offset_fontpix + corners_fontpix[i];
        gl_Position = u_clip_from_fontpix * vec4(corner_fontpix, pos_fontpix[0].z, 1.0);
        v_uv = font_offset_uv.xy + corners_fontpix[i] / textureSize(u_font_atlas, 0);
        EmitVertex();
    }

    EndPrimitive();
}

@start fragment
#version 150 core
#include "utils.glsl.h"
#include "sdf.glsl.h"

uniform sampler2D u_font_atlas;

in vec2  v_uv;
out vec4 FragColor;

uniform int  u_font_bitmap_type;
uniform vec2  u_max_sdf_dist_uv;
uniform vec3  u_color;

float SdfScaleFactor(vec2 tex_uv, vec2 unit_range) {
    vec2 screenTexSize = vec2(1.0)/fwidth(tex_uv);
    return max(0.5*dot(unit_range, screenTexSize), 1.0);
}

void main() {
    vec3 texel = texture(u_font_atlas, v_uv).xyz;
    float opacity;

    if(u_font_bitmap_type == 0) {
        opacity = texel.r;
    }else{
        float sd = (u_font_bitmap_type == 1) ? texel.r: median(texel.r, texel.g, texel.b);
        float sdf = SdfScaleFactor(v_uv, u_max_sdf_dist_uv)*(sd - 0.5);
        opacity = clamp( sdf + 0.5, 0.0, 1.0);
    }

    FragColor = vec4( u_color, opacity );
}
