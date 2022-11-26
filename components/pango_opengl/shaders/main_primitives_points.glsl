@start vertex
#version 330 core
layout(location = 0) in vec3 a_position;
uniform mat4 proj;
uniform mat4 cam_from_world;

void main() {
    gl_Position = proj * cam_from_world * vec4(a_position, 1.0);
}

@start fragment
#version 330 core

uniform vec4 color;
out vec4 FragColor;

void main() {
    FragColor = vec4(color);
}
