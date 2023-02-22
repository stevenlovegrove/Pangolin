#pragma once

#include <pangolin/drawable/drawable.h>
#include <pangolin/maths/conventions.h>
#include <pangolin/render/colormap.h>
#include <pangolin/render/device_texture.h>
#include <sophus/image/dyn_image_types.h>

namespace pangolin
{

// Just renders a fixed checker as a background for image layers to help
// distinguish background from image
struct DrawnChecker : public Drawable {
  struct Params {
    Eigen::Vector4f check_color_1 = {0.5, 0.5, 0.5, 1.0};
    Eigen::Vector4f check_color_2 = {0.8, 0.8, 0.8, 1.0};
    int check_size_pixels = 10;
  };
  static Shared<DrawnChecker> Create(Params p);
};

}  // namespace pangolin
