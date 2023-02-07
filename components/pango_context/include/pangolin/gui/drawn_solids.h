#pragma once

#include <pangolin/gui/draw_layer.h>
#include <pangolin/maths/conventions.h>
#include <pangolin/render/colormap.h>
#include <pangolin/render/device_buffer.h>
#include <pangolin/render/device_texture.h>
#include <sophus/image/runtime_image_types.h>

#include <vector>

namespace pangolin
{

struct DrawnSolids : public Drawable {
  enum class Type { checkerboard };

  Type object_type;

  struct Params {
    Type object_type = Type::checkerboard;
  };
  static Shared<DrawnSolids> Create(Params p);
};

}  // namespace pangolin
