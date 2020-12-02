#include <pangolin/display/default_font.h>

#include "pangolin_gl.h"

extern const unsigned char AnonymousPro_ttf[];

namespace pangolin {

GlFont& default_font()
{
    PangolinGl* context = GetCurrentContext();
    PANGO_ASSERT(context);
    if(!context->font) {
        context->font = std::make_shared<GlFont>(AnonymousPro_ttf, 18);
    }
    return *(context->font.get());
}

}
