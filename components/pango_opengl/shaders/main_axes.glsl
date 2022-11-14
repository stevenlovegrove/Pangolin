@start vertex
#version 330 core
layout(location = 0) in vec4 a_xyzw_quat;
layout(location = 1) in vec4 a_txtytz_;
uniform mat4 cam_from_world;
out vec4 center_world;
out vec4 quaternion;

void main() {
    center_world = cam_from_world * vec4(a_txtytz_.xyz, 1.0);
    quaternion = a_xyzw_quat;
}

@start geometry
#version 330
layout (points) in;
layout (line_strip, max_vertices = 5) out;

#include "quaternion.glsl.h"

in vec4 center_world[];
in vec4 quaternion[];
out vec4 vert_color;

uniform mat4 proj;
uniform vec4 color;
uniform float length;

void main() {
    mat3 R = mat3FromQuaternion(quaternion[0]);
    vec4 center = proj * vec4(center_world[0].xyz, 1.0);

    vert_color = vec4(1.0, 0.0, 0.0, 1.0);
    gl_Position = proj * vec4(center_world[0].xyz + length*R[0], 1.0);
    EmitVertex();
    vert_color = color;
    gl_Position = center;
    EmitVertex();
    vert_color = vec4(0.0, 1.0, 0.0, 1.0);
    gl_Position = proj * vec4(center_world[0].xyz + length*R[1], 1.0);
    EmitVertex();
    vert_color = color;
    gl_Position = center;
    EmitVertex();
    vert_color = vec4(0.0, 0.0, 1.0, 1.0);
    gl_Position = proj * vec4(center_world[0].xyz + length*R[2], 1.0);
    EmitVertex();
}

@start fragment
#version 330 core

in vec4 vert_color;
out vec4 FragColor;

void main() {
    FragColor = vec4(vert_color);
}
