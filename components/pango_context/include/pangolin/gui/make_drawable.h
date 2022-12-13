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

// https://en.wikipedia.org/w/index.php?title=Geodesic_polyhedron&oldid=1011674560
struct Icosphere {
  float radius = 1.0;
  Color color = Color::white();
  size_t num_subdivisions = 2u;
};

struct CheckerPlane {
};

struct Axis {
  float scale;
  float line_width;
};

struct Axes {
 static Axes from(
      std::vector<sophus::SE3d> const& entity_poses_axis_d,
      float scale = 0.1f,
      float line_width = 1.5f) {
    Axes axes{.scale = scale,.line_width=line_width};
    for (auto drawable_from_axis : axes.drawable_from_axis_poses) {
      axes.drawable_from_axis_poses.push_back(drawable_from_axis.cast<float>());
    }
    return axes;
  }

  static Axes from(
      std::vector<sophus::SE3f> const& entity_poses_axis_d,
      float scale = 0.1f,
      float line_width = 1.5f) {
    Axes axes{.scale = scale,.line_width=line_width};
    axes.drawable_from_axis_poses = entity_poses_axis_d;
    return axes;
  }

  std::vector<sophus::SE3f> drawable_from_axis_poses;
  float scale = 0.1f;
  float line_width = 1.5;
};
}

template<>
struct DrawableConversionTraits<Draw::Shape> {
static Shared<Drawable> makeDrawable(const Draw::Shape& x);
};


template<>
struct DrawableConversionTraits<Draw::Cube> {
static Shared<Drawable> makeDrawable(const Draw::Cube& x);
};

template<>
struct DrawableConversionTraits<Draw::Icosphere> {
static Shared<Drawable> makeDrawable(const Draw::Icosphere& x);
};

template<>
struct DrawableConversionTraits<Draw::CheckerPlane> {
static Shared<Drawable> makeDrawable(const Draw::CheckerPlane& x);
};

template<>
struct DrawableConversionTraits<Draw::Axes> {
static Shared<Drawable> makeDrawable(const Draw::Axes& x);
};
// Draw::Axes convenient methods

// Single axis at the identity.
//
// Example: scene->addToSceneAt(Draw::Axis{.size - 0.5}, sophus::SE3d{...});
template<>
struct DrawableConversionTraits<Draw::Axis> {
static Shared<Drawable> makeDrawable(const Draw::Axis& x);
};

template<>
struct DrawableConversionTraits<sophus::Se3F32> {
static Shared<Drawable> makeDrawable(const sophus::Se3F32& x) {
    Draw::Axes axes;
    axes.drawable_from_axis_poses.push_back(x);
    return DrawableConversionTraits<Draw::Axes>::makeDrawable(axes);
}
};

template<typename T>
struct DrawableConversionTraits<sophus::Se3<T>> {
static Shared<Drawable> makeDrawable(const sophus::Se3<T>& x) {
    return makeDrawable(x.template cast<float>());
}
};


}
