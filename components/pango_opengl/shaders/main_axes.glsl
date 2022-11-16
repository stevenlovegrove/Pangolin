@start vertex
#version 330 core
#include "lie_groups.glsl.h"

layout(location = 0) in vec4 a_quat_world_from_axes;
layout(location = 1) in vec4 a_axes_in_world; // .w is junk padding
uniform mat4 cam_from_world;
out mat4 cam_T_axes;

void main() {
    cam_T_axes = cam_from_world *
        makeSe3(a_quat_world_from_axes, a_axes_in_world.xyz);
}

@start geometry
#version 330
layout (points) in;
layout (line_strip, max_vertices = 5) out;

in mat4 cam_T_axes[];
out vec4 vert_color;

uniform mat4 proj;
uniform vec4 color;
uniform float length;

void main() {
    mat4 points_axes = mat4(
        vec4(length, 0.0, 0.0, 1.0), // x column vec
        vec4(0.0, length, 0.0, 1.0), // y column vec
        vec4(0.0, 0.0, length, 1.0), // z column vec
        vec4(0.0, 0.0, 0.0, 1.0)     // center vec
    );

    // Transform to cam frame and project
    mat4 points_proj = proj * cam_T_axes[0] * points_axes;

    vert_color = vec4(1.0, 0.0, 0.0, 1.0);
    gl_Position = points_proj[0];
    EmitVertex();
    vert_color = color;
    gl_Position = points_proj[3];
    EmitVertex();
    vert_color = vec4(0.0, 1.0, 0.0, 1.0);
    gl_Position = points_proj[1];
    EmitVertex();
    vert_color = color;
    gl_Position = points_proj[3];
    EmitVertex();
    vert_color = vec4(0.0, 0.0, 1.0, 1.0);
    gl_Position = points_proj[2];
    EmitVertex();
}

@start fragment
#version 330 core

in vec4 vert_color;
out vec4 FragColor;

void main() {
    FragColor = vec4(vert_color);
}
