#pragma once

#include <pangolin/gl/color.h>
#include <pangolin/gui/draw_layer.h>
#include <pangolin/gui/drawn_primitives.h>

namespace pangolin
{

namespace draw
{
struct Shape {
  Eigen::Vector3d pos = {0.0, 0.0, 0.0};
  Eigen::Vector4d color = {1.0, 0.3, 0.5, 1.0};
  double size = 1.0;
  DrawnPrimitives::Shape type = DrawnPrimitives::Shape::hollow_star;
};

struct Cube {
  float size = 1.f;
  std::array<Color, 6> colors = {Color::red(),   Color::red(),  Color::green(),
                                 Color::green(), Color::blue(), Color::blue()};
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
  float scale = 0.1f;
  float line_width = 1.5;
};

struct Axes {
  static Axes from(
      const std::vector<sophus::SE3d>& entity_poses_axis_d, float scale = 0.1f,
      float line_width = 1.5f)
  {
    Axes axes{.scale = scale, .line_width = line_width};
    for (auto drawable_from_axis : axes.drawable_from_axis_poses) {
      axes.drawable_from_axis_poses.push_back(drawable_from_axis.cast<float>());
    }
    return axes;
  }

  static Axes from(
      const std::vector<sophus::SE3f>& entity_poses_axis_d, float scale = 0.1f,
      float line_width = 1.5f)
  {
    Axes axes{.scale = scale, .line_width = line_width};
    axes.drawable_from_axis_poses = entity_poses_axis_d;
    return axes;
  }

  std::vector<sophus::SE3f> drawable_from_axis_poses;
  float scale = 0.1f;
  float line_width = 1.5;
};

template <class T>
struct Points3 {
  std::vector<Eigen::Vector<T, 3>> points;
  Color color;
  float size = 1.5f;
};

using Points3f = Points3<float>;
using Points3d = Points3<double>;

struct Line2 {
  Line2() {}
  Line2(Eigen::Vector2f p0, Eigen::Vector2f p1, Color color = Color::white()) :
      a(p0), b(p1), color(color)
  {
  }
  Line2(Eigen::Vector2d p0, Eigen::Vector2d p1, Color color = Color::white()) :
      a(p0.cast<float>()), b(p1.cast<float>()), color(color)
  {
  }

  Eigen::Vector3f toXy0A() const { return Eigen::Vector3f(a.x(), a.y(), 0.f); }

  Eigen::Vector3f toXy0B() const { return Eigen::Vector3f(b.x(), b.y(), 0.f); }

  Eigen::Vector2f a = Eigen::Vector2f::Zero();
  Eigen::Vector2f b = Eigen::Vector2f::Zero();
  Color color = Color::white();
  float line_width = 3;
};

struct Line3 {
  Line3() {}

  Line3(Eigen::Vector3f p0, Eigen::Vector3f p1) : a(p0), b(p1) {}
  Line3(Eigen::Vector3f p0, Eigen::Vector3f p1, Color color) : Line3(p0, p1)
  {
    this->color = color;
  }

  Line3(Eigen::Vector3d p0, Eigen::Vector3d p1) :
      Line3(p0.cast<float>().eval(), p1.cast<float>().eval())
  {
  }
  Line3(Eigen::Vector3d p0, Eigen::Vector3d p1, Color color) :
      Line3(p0.cast<float>().eval(), p1.cast<float>().eval(), color)
  {
  }

  Eigen::Vector3f a = Eigen::Vector3f::Zero();
  Eigen::Vector3f b = Eigen::Vector3f::Zero();
  Color color = Color::white();
  float line_width = 3;
};

struct CameraFrustum {
  sophus::CameraModel camera;
  float near = 0.01;
  float far = 1.0;
  Color color = Color::red();
};

struct Circle3 {
  Eigen::Vector3d center = Eigen::Vector3d::Zero();
  double radius = 1.0;
  Eigen::Vector3d a = Eigen::Vector3d::Zero();
  Eigen::Vector3d b = Eigen::Vector3d::Zero();
  Color color = Color::white();
};

}  // namespace draw

template <>
struct DrawableConversionTraits<draw::Shape> {
  static Shared<Drawable> makeDrawable(const draw::Shape& x);
};

template <>
struct DrawableConversionTraits<draw::Cube> {
  static Shared<Drawable> makeDrawable(const draw::Cube& x);
};

template <>
struct DrawableConversionTraits<draw::Icosphere> {
  static Shared<Drawable> makeDrawable(const draw::Icosphere& x);
};

template <>
struct DrawableConversionTraits<draw::CheckerPlane> {
  static Shared<Drawable> makeDrawable(const draw::CheckerPlane& x);
};

template <>
struct DrawableConversionTraits<draw::Axes> {
  static Shared<Drawable> makeDrawable(const draw::Axes& x);
};
// draw::Axes convenient methods

template <>
struct DrawableConversionTraits<sophus::Se3F32> {
  static Shared<Drawable> makeDrawable(const sophus::Se3F32& x)
  {
    draw::Axes axes;
    axes.drawable_from_axis_poses.push_back(x);
    return DrawableConversionTraits<draw::Axes>::makeDrawable(axes);
  }
};

template <typename T>
struct DrawableConversionTraits<sophus::Se3<T>> {
  static Shared<Drawable> makeDrawable(const sophus::Se3<T>& x)
  {
    return makeDrawable(x.template cast<float>());
  }
};

// Single axis at the identity.
//
// Example: scene->addToSceneAt(draw::Axis{.size - 0.5}, sophus::SE3d{...});
template <>
struct DrawableConversionTraits<draw::Axis> {
  static Shared<Drawable> makeDrawable(const draw::Axis& x)
  {
    draw::Axes axes;
    axes.line_width = x.line_width;
    axes.scale = x.scale;
    axes.drawable_from_axis_poses.push_back(sophus::SE3f{});
    return DrawableConversionTraits<draw::Axes>::makeDrawable(axes);
  }
};

template <>
struct DrawableConversionTraits<draw::Points3f> {
  static Shared<Drawable> makeDrawable(const draw::Points3f& x);
};

template <>
struct DrawableConversionTraits<draw::Points3d> {
  static Shared<Drawable> makeDrawable(const draw::Points3d& x);
};

template <>
struct DrawableConversionTraits<std::vector<draw::Line3>> {
  static Shared<Drawable> makeDrawable(const std::vector<draw::Line3>& x);
};

template <>
struct DrawableConversionTraits<std::vector<draw::Line2>> {
  static Shared<Drawable> makeDrawable(const std::vector<draw::Line2>& x);
};

template <>
struct DrawableConversionTraits<draw::CameraFrustum> {
  static Shared<Drawable> makeDrawable(const draw::CameraFrustum& x);
};

template <>
struct DrawableConversionTraits<draw::Circle3> {
  static Shared<Drawable> makeDrawable(const draw::Circle3& x);
};

}  // namespace pangolin
