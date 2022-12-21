#include "pangolin_gl.h"

#include <pangolin/display/default_font.h>

extern unsigned char const AnonymousPro_ttf[];

namespace pangolin
{

std::shared_ptr<GlFont> build_builtin_font(
    float pixel_height, int tex_w, int tex_h, bool use_alpha_font)
{
  return std::make_shared<GlFont>(
      AnonymousPro_ttf, pixel_height, tex_w, tex_h, use_alpha_font);
}

std::shared_ptr<GlFont> default_font()
{
  PangolinGl* context = GetCurrentContext();
  PANGO_ASSERT(context);
  if (!context->font) {
    context->font = build_builtin_font(18);
  }
  return context->font;
}

}  // namespace pangolin
