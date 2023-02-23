@start vertex
#version 330 core

layout(location = 0) in vec3 a_position;

#expect VERTEX_COLORS
#if VERTEX_COLORS
    layout(location = 1) in vec4 a_colors;
    out vec4 color;
#endif

#expect VERTEX_NORMALS
#if VERTEX_NORMALS
    layout(location = 2) in vec3 a_normals_world;
    out vec3 normal_cam;
#endif

#expect VERTEX_UVS
#if VERTEX_UVS
    layout(location = 3) in vec2 a_uvs;
    out vec2 uv;
#endif

uniform mat4 proj;
uniform mat4 cam_from_world;

void main() {
    gl_Position = proj * cam_from_world * vec4(a_position, 1.0);
    #if VERTEX_COLORS
        color = a_colors;
    #endif
    #if VERTEX_NORMALS
        normal_cam = normalize(mat3(cam_from_world) * a_normals_world);
    #endif
    #if VERTEX_UVS
        uv = a_uvs;
    #endif
}

@start fragment
#version 330 core

#expect USE_TEXTURE
#if USE_TEXTURE
    uniform sampler2D texture_for_uv;
#endif

#expect USE_MATCAP
#if USE_MATCAP
    uniform sampler2D texture_matcap;
#endif

#expect VERTEX_COLORS
#if VERTEX_COLORS
    in vec4 color;
#else
    uniform vec4 color;
#endif

#expect VERTEX_NORMALS
#if VERTEX_NORMALS
    in vec3 normal_cam;
#endif

#expect VERTEX_UVS
#if VERTEX_UVS
    in vec2 uv;
#endif



out vec4 FragColor;

void main() {
    #if USE_MATCAP && VERTEX_NORMALS
        vec2 matcap_uv = (normal_cam.xy + vec2(1.0)) / 2.0;
        FragColor = texture(texture_matcap, matcap_uv );
    #elif USE_TEXTURE && VERTEX_UVS
        FragColor = texture(texture_for_uv, uv);
    #elif VERTEX_NORMALS
        FragColor = vec4(normal_cam, 1.0);
    #else
        FragColor = vec4(color);
    #endif
}
