@start vertex
#version 330 core

layout(location = 0) in vec3 a_position;

#expect VERTEX_COLORS
#if VERTEX_COLORS
    layout(location = 1) in vec4 a_colors;
    out vec4 color;
#endif

uniform mat4 proj;
uniform mat4 cam_from_world;

void main() {
    gl_Position = proj * cam_from_world * vec4(a_position, 1.0);
    #if VERTEX_COLORS
        color = a_colors;
    #endif
}

@start fragment
#version 330 core

#expect VERTEX_COLORS
#if VERTEX_COLORS
    in vec4 color;
#else
    uniform vec4 color;
#endif

out vec4 FragColor;

void main() {
    FragColor = vec4(color);
}
