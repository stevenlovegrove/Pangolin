@start vertex
#version 150 core

out vec2 v_tex;

const vec2 pos[4] = vec2[4](
  vec2( -1.0, +1.0), vec2( -1.0, -1.0),
  vec2( +1.0, +1.0), vec2( +1.0, -1.0)
);

void main()
{
  gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);
  v_tex = (pos[gl_VertexID] * vec2(1.0,-1.0) + vec2(1.0)) / 2.0;
}

@start fragment
#version 150 core

#include </components/pango_opengl/shaders/camera.glsl.h>
#include </components/pango_opengl/shaders/geom.glsl.h>
#include </components/pango_opengl/shaders/grid.glsl.h>

in vec2 v_tex;
out vec4 color;

uniform mat3 kinv;
uniform mat4 world_from_cam;
uniform vec2 image_size;
uniform vec2 znear_zfar;

vec4 color_sky = vec4(1.0,1.0,1.0,1.0);

void main()
{
  mat3 w_R_c = mat3(world_from_cam);
  vec3 c_w = world_from_cam[3].xyz;

  vec2 pixel = getCameraPixelCoord(image_size, v_tex);
  vec3 ray_in_cam = unproj(kinv, pixel);
  vec3 ray_in_world = w_R_c * ray_in_cam;
  float depth = intersectRayPlaneZ0(c_w, ray_in_world);

  if(depth < 0.0) {
    color = color_sky;
    gl_FragDepth = 0.0;
    return;
  }

  vec3 p_in_world = c_w + depth * ray_in_world;

  vec2 p_grid = p_in_world.xy*5.0;
  float check = 0.8+0.2*checkerFilteredFaded(p_grid, depth, 10.0);

  color = vec4(vec3(check),1.0);
  gl_FragDepth = fragDepthFromSceneDepth(depth, znear_zfar.x, znear_zfar.y);
}
