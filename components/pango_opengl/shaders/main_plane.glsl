@start vertex
#version 150 core

out vec2 v_clip_pos;

const vec2 pos[4] = vec2[4](
  vec2( -1.0, +1.0), vec2( -1.0, -1.0),
  vec2( +1.0, +1.0), vec2( +1.0, -1.0)
);

void main()
{
  gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);
  v_clip_pos = pos[gl_VertexID];
}

@start fragment
#version 150 core

#include </components/pango_opengl/shaders/utils.glsl.h>
#include </components/pango_opengl/shaders/camera.glsl.h>
#include </components/pango_opengl/shaders/geom.glsl.h>
#include </components/pango_opengl/shaders/grid.glsl.h>

in vec2 v_clip_pos;
out vec4 color;

uniform mat4 camera_from_clip;
uniform mat4 world_from_cam;
uniform vec2 znear_zfar;

vec4 color_sky = vec4(1.0,1.0,1.0,1.0);

vec4 xGround(vec3 c_w, vec3 dir_w)
{
  return vec4( intersectRayPlaneZ0(c_w, dir_w), vec3(0.0, 0.0, 1.0) );
}

void updateDepthNormal(
  inout vec4 albedo,  inout vec4 dnorm,
  vec4 sample_albedo, vec4 sample_dnorm
) {
  if (sample_dnorm.x >= 0 && sample_dnorm.x < dnorm.x) {
    dnorm = sample_dnorm;
    albedo = sample_albedo;
  }
}

vec4 material(vec4 albedo, vec4 depth_normal)
{
  return vec4(vec3(albedo), 1.0);
}

void main()
{
  mat3 w_R_c = mat3(world_from_cam);
  vec3 c_w = world_from_cam[3].xyz;

  vec3 dir_c = normToOneZ(proj(camera_from_clip * vec4(v_clip_pos, -1.0, 1.0)));
  float dir_c_len = length(dir_c);
  vec3 dir_w = w_R_c * dir_c / dir_c_len;

  vec4 albedo = color_sky;
  vec4 depth_normal = vec4(znear_zfar.y, 0, 0, 0);
  vec4 depth_normal_ground = xGround(c_w, dir_w);
  vec3 ground_pos = c_w + dir_w * depth_normal_ground.x;
  float check_pattern = checkerFilteredFaded(ground_pos.xy, depth_normal_ground.x, 50.0);
  vec4 albedo_ground = vec4(0.8 + 0.2 * vec3(check_pattern), 1.0);

  updateDepthNormal(albedo, depth_normal, albedo_ground, depth_normal_ground );

  // set the end color and depth. We have to convert from radial depth to
  // z-depth through division with dir_c_len
  color = material(albedo, depth_normal);
  gl_FragDepth = fragDepthFromSceneDepth(depth_normal.x/dir_c_len, znear_zfar.x, znear_zfar.y);
}
