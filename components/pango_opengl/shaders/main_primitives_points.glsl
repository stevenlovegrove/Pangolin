@start vertex
#version 330 core
layout(location = 0) in vec3 a_position;
uniform mat4 proj;
uniform mat4 cam_from_world;

void main() {
    // vec4 p_cam = cam_from_world * vec4(a_position, 1.0);
    // gl_Position = proj * p_cam;
    gl_Position = proj * cam_from_world * vec4(a_position, 1.0);
}

@start fragment
#version 330 core

uniform float u_time;
out vec4 FragColor;

vec3 colorA = vec3(0.905,0.045,0.045);
vec3 colorB = vec3(0.995,0.705,0.051);

void main() {
    FragColor = vec4(colorA, 1.0);
}
