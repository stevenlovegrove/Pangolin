#include "font_offset_table.glsl.h"

float screenPxRange(vec2 tex_coord)
{
  float const pxRange = 2.0;
  vec2 unitRange = vec2(pxRange) / vec2(textureSize(u_font_atlas, 0));
  vec2 screenTexSize = vec2(1.0) / fwidth(tex_coord);
  return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

float sdf_font(vec4 atlas_offset, vec2 pos)
{
  vec2 atlas_dim = textureSize(u_font_atlas, 0);
  vec2 p = clamp2(
      pos / atlas_dim, vec2(0.0, 0.0), vec2(atlas_offset.z, atlas_offset.w));

  vec2 uv = atlas_offset.xy + p;
  vec3 msd = texture(u_font_atlas, uv).xyz;
  float sd = median(msd.r, msd.g, msd.b);
  return screenPxRange(uv) * (sd - 0.5);
}

float sdf_font(int char_id, vec2 pos)
{
  vec4 font_offset = texelFetch(u_font_offsets, ivec2(char_id, 0), 0);
  vec2 screen_offset = texelFetch(u_font_offsets, ivec2(char_id, 1), 0).xy;
  return sdf_font(font_offset, pos);
}

float font_color(vec4 atlas_offset, vec2 pos)
{
  return clamp(sdf_font(atlas_offset, pos) + 0.5, 0.0, 1.0);
}

float font_color(int char_id, vec2 pos)
{
  return clamp(sdf_font(char_id, pos) + 0.5, 0.0, 1.0);
}
