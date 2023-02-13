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

uniform float size;
uniform mat4 proj;
uniform mat4 cam_from_world;

void main()
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
  float sdf = (abs(val)-0.5) / fwidth(val);
//   float sdf = abs(val) / fwidth(val) - size;

  FragColor = color_sdf(sdf, color);
//   FragColor = vec4(vec3(abs(1.0 -sdf)),1.0);
}
