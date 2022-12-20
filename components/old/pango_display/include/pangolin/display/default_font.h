#pragma once

#include <pangolin/gl/glfont.h>

#include <memory>

namespace pangolin
{

// Try not to use these methods. They'll likely change in the future.

// GlFont currently holds a rasterized font atlas.
// pixel_height is the canonical size of a charector in the atlas in pixels
// tex_w, tex_h are the atlas dimensions. The method will throw if tex_w and
// tex_h are too small to fit the font use_alpha_font - choose to render as a
// bitmap with alpha channel, or if false, as an SDF.
std::shared_ptr<GlFont> build_builtin_font(
    float pixel_height, int tex_w = 1024, int tex_h = 1024,
    bool use_alpha_font = true);

std::shared_ptr<GlFont> default_font();

}  // namespace pangolin
