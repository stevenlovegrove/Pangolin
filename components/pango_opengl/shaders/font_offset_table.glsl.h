uniform sampler2D u_font_offsets;

void font_and_screen_offset(
    uint char_id, out vec4 font_offset, out vec4 screen_offset)
{
  font_offset = texelFetch(u_font_offsets, ivec2(char_id, 0), 0);
  screen_offset = texelFetch(u_font_offsets, ivec2(char_id, 1), 0);
}
