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
uniform float tick_to_tick;
uniform vec2 graph_per_pix;
uniform vec2 log10_view;
uniform vec2 log10_start;
uniform vec2 unit_graph_start;

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
  vec2 unit_graph = unit_graph_start;

  color = color_background;

  for(int i=0; i < 4; ++i) {
    vec2 dist_pix = modDist(graph, unit_graph) / graph_per_pix;
    vec2 logdiff = log10_start - log10_view; // [0-1].
    vec4 attenuation_x = (i==0) ? vec4(1.0) - one_minus_atten * logdiff.x : atten;
    vec4 attenuation_y = (i==0) ? vec4(1.0) - one_minus_atten * logdiff.y : atten;
    color *= transition(attenuation_x, vec4(1.0), 1.5, dist_pix.x);
    color *= transition(attenuation_y, vec4(1.0), 1.5, dist_pix.y);
    unit_graph *= tick_to_tick;
  }
}
