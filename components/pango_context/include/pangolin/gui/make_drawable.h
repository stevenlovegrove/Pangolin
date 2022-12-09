#pragma once

#include <pangolin/gui/draw_layer.h>
#include <pangolin/gui/drawn_primitives.h>
#include <pangolin/gl/color.h>

namespace pangolin{

namespace Draw{
struct Shape
{
    Eigen::Vector3d pos = {0.0, 0.0, 0.0};
    Eigen::Vector4d color = {1.0, 0.3, 0.5, 1.0};
    double size = 1.0;
    DrawnPrimitives::Shape type = DrawnPrimitives::Shape::hollow_star;
};

struct Cube {

  float size = 1.f;
  std::array<Color, 6> colors = {
      Color::red(),
      Color::red(),
      Color::green(),
      Color::green(),
      Color::blue(),
      Color::blue()};
};
}

template<>
struct DrawableConversionTraits<Draw::Shape> {
static Shared<Drawable> makeDrawable(const Draw::Shape& x);
};

template<>
struct DrawableConversionTraits<sophus::Se3F32> {
static Shared<Drawable> makeDrawable(const sophus::Se3F32& x);
};


template<typename T>
struct DrawableConversionTraits<sophus::Se3<T>> {
static Shared<Drawable> makeDrawable(const sophus::Se3<T>& x) {
    return makeDrawable(x.template cast<float>());
}
};


}