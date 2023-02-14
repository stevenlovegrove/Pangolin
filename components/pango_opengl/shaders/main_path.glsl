@start vertex
#version 330 core

layout(location = 0) in vec3 a_pos0;
layout(location = 1) in vec3 a_pos1;
layout(location = 2) in vec3 a_pos2;
layout(location = 3) in vec3 a_pos3;

#expect VERTEX_COLORS
#if VERTEX_COLORS
    layout(location = 4) in vec4 a_colors;
    out vec4 color;
#endif

out vec3 pos0;
out vec3 pos1;
out vec3 pos2;
out vec3 pos3;

void main() {
    pos0 = a_pos0;
    pos1 = a_pos1;
    pos2 = a_pos2;
    pos3 = a_pos3;

    #if VERTEX_COLORS
        color = a_colors;
    #endif
}

@start geometry
#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in vec3 pos0[];
in vec3 pos1[];
in vec3 pos2[];
in vec3 pos3[];
out float val;

uniform mat4 proj;
uniform mat4 cam_from_world;

uniform float size;
uniform vec2 size_clip;

// 0: Oriented/Sized in XY World plane
// 1: Oriented/Sized in Clip plane.
uniform int mode;

vec3 dehomog(vec4 h) {
    return h.xyz / h.w;
}

void main_pixels()
{
    vec3 clip0 = dehomog(proj * cam_from_world * vec4(pos0[0], 1.0));
    vec3 clip1 = dehomog(proj * cam_from_world * vec4(pos1[0], 1.0));
    vec3 clip2 = dehomog(proj * cam_from_world * vec4(pos2[0], 1.0));
    vec3 clip3 = dehomog(proj * cam_from_world * vec4(pos3[0], 1.0));

    vec2 linem_dir = normalize(clip1.xy - clip0.xy);
    vec2 line0_dir = normalize(clip2.xy - clip1.xy);
    vec2 linep_dir = normalize(clip3.xy - clip2.xy);

    vec2 norm_m = linem_dir.yx * vec2(1.0, -1.0);
    vec2 norm_0 = line0_dir.yx * vec2(1.0, -1.0);
    vec2 norm_p = linep_dir.yx * vec2(1.0, -1.0);

    vec2 n1 = (norm_m+norm_0)/2.0;
    vec2 n2 = (norm_0+norm_p)/2.0;

    float s1 = 1.0 / dot(n1,norm_0);
    float s2 = 1.0 / dot(n2,norm_0);

    vec3 offset1 = vec3(s1*n1,0.0);
    vec3 offset2 = vec3(s2*n2,0.0);
    offset1.xy *=size_clip;
    offset2.xy *=size_clip;

    val = +1.0;
    gl_Position = vec4(clip1 + offset1, 1.0);
    EmitVertex();
    val = -1.0;
    gl_Position = vec4(clip1 - offset1, 1.0);
    EmitVertex();
    val = +1.0;
    gl_Position = vec4(clip2 + offset2, 1.0);
    EmitVertex();
    val = -1.0;
    gl_Position = vec4(clip2 - offset2, 1.0);
    EmitVertex();

   EndPrimitive();
}

void main_scene()
{
    vec2 linem_dir = normalize(pos1[0].xy - pos0[0].xy);
    vec2 line0_dir = normalize(pos2[0].xy - pos1[0].xy);
    vec2 linep_dir = normalize(pos3[0].xy - pos2[0].xy);

    vec2 norm_m = linem_dir.yx * vec2(1.0, -1.0);
    vec2 norm_0 = line0_dir.yx * vec2(1.0, -1.0);
    vec2 norm_p = linep_dir.yx * vec2(1.0, -1.0);

    vec2 n1 = (norm_m+norm_0)/2.0;
    vec2 n2 = (norm_0+norm_p)/2.0;

    float s1 = size / dot(n1,norm_0);
    float s2 = size / dot(n2,norm_0);

    vec3 offset1 = vec3(s1*n1,0.0);
    vec3 offset2 = vec3(s2*n2,0.0);

    val = +1.0;
    gl_Position = proj * cam_from_world * vec4(pos1[0] + offset1, 1.0);
    EmitVertex();
    val = -1.0;
    gl_Position = proj * cam_from_world * vec4(pos1[0] - offset1, 1.0);
    EmitVertex();
    val = +1.0;
    gl_Position = proj * cam_from_world * vec4(pos2[0] + offset2, 1.0);
    EmitVertex();
    val = -1.0;
    gl_Position = proj * cam_from_world * vec4(pos2[0] - offset2, 1.0);
    EmitVertex();

   EndPrimitive();
}

void main()
{
    if(mode == 1) {
        main_pixels();
    }else{
        main_scene();
    }
}

@start fragment
#version 330 core

#include "sdf.glsl.h"

in float val;

#expect VERTEX_COLORS
#if VERTEX_COLORS
    in vec4 color;
#else
    uniform vec4 color;
#endif

out vec4 FragColor;

uniform float size;

void main() {
  float sdf = (abs(val)-0.95) / fwidth(val);
//   float sdf = abs(val) / fwidth(val) - size;

  FragColor = color_sdf(sdf, color);
//   FragColor = vec4(vec3(abs(1.0 -sdf)),1.0);
}
