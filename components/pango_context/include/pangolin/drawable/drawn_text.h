#pragma once

#include <pangolin/layer/draw_layer.h>

namespace pangolin
{

struct DrawnText : public Drawable {
  virtual void addText(
      const Eigen::Vector2d& pos_in_drawable, const std::string& utf8,
      double angle = 0, const double font_size_em = 1.0) = 0;

  virtual void clearTexts() = 0;

  struct Params {
    size_t font_height_pixels = 32;
  };
  static Shared<DrawnText> Create(Params p);
};

}  // namespace pangolin
