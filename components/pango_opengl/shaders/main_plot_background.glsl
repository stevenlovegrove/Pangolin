@start vertex
#version 330 core

uniform vec2 viewport_size;
uniform mat4 graph_from_clip;

out vec2 graph;

const vec2 pos[4] = vec2[4](
  vec2( -1.0, +1.0), vec2( -1.0, -1.0),
  vec2( +1.0, +1.0), vec2( +1.0, -1.0)
);

void main()
{
  vec2 clip = pos[gl_VertexID];
  gl_Position = vec4(clip, 0.0, 1.0);

  graph = (graph_from_clip * gl_Position).xy;
}

@start fragment
#version 330 core

#include <sdf.glsl.h>

in vec2 graph;

uniform vec4 color_background;
uniform vec4 tick_color_scale;
uniform float num_divisions;
uniform vec2 graph_units_per_pixel;
uniform vec2 log_s_min_dist;
uniform vec2 log_s_of_octave_start;
uniform vec2 octave_start;

out vec4 color;

float modDist(float v, float modval) {
  float dist = mod(v, modval);
  return min(dist, modval - dist);
}

vec2 modDist(vec2 v, vec2 modval)
{
  return vec2(
    modDist(v.x, modval.x),
    modDist(v.y, modval.y)
  );
}

void main() {
  vec4 atten = tick_color_scale;
  vec4 one_minus_atten = vec4(1.0) - tick_color_scale;
  vec2 ith_octave_start = octave_start;

  color = color_background;

  int num_octaves = 4;
  for(int i=0; i < num_octaves; ++i) {
    vec2 dist_pix = modDist(graph, ith_octave_start) / graph_units_per_pixel;
    vec2 logdiff = log_s_of_octave_start - log_s_min_dist; // [0-1].
    vec4 attenuation_x = (i==0) ? vec4(1.0) - one_minus_atten * logdiff.x : atten;
    vec4 attenuation_y = (i==0) ? vec4(1.0) - one_minus_atten * logdiff.y : atten;
    color *= transition(attenuation_x, vec4(1.0), 1.5, dist_pix.x);
    color *= transition(attenuation_y, vec4(1.0), 1.5, dist_pix.y);
    ith_octave_start *= num_divisions;
  }
}
