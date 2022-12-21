#pragma once

#include <pangolin/gui/draw_layer_handler.h>
#include <pangolin/gui/drawable.h>
#include <pangolin/gui/drawn_checker.h>
#include <pangolin/gui/layer_group.h>
#include <pangolin/maths/camera_look_at.h>
#include <pangolin/maths/min_max.h>
#include <sophus/lie/se3.h>
#include <sophus/lie/sim2.h>
#include <sophus/sensor/camera_model.h>

#include <variant>

namespace pangolin
{

template <typename T>
struct DrawableConversionTraits;

struct DeviceTexture;

enum AspectPolicy {
  stretch,  // bounds stretch to viewport edges
  crop,     // viewport is cropped to maintain bounds aspect
  overdraw  // View matrices extended to maintain aspect but fill viewport.
};

struct DrawLayerRenderState {
  sophus::CameraModel camera;
  sophus::SE3d camera_from_world;
  sophus::Sim2<double> clip_view_transform;
  MinMax<double> near_far;

  AspectPolicy aspect_policy;
  ImageXy image_convention;
  ImageIndexing image_indexing;
};

////////////////////////////////////////////////////////////////////
/// Supports displaying interactive 2D / 3D elements
///
struct DrawLayer : public Layer {
  enum In { scene, pixels };

  virtual DrawLayerRenderState const& renderState() const = 0;
  virtual void setCamera(sophus::CameraModel const&) = 0;
  virtual void setCameraFromWorld(sophus::Se3<double> const&) = 0;
  virtual void setClipViewTransform(sophus::Sim2<double>&) = 0;
  virtual void setNearFarPlanes(MinMax<double> const&) = 0;
  virtual void add(
      Shared<Drawable> const& r, In domain, std::string const& name = "") = 0;
  virtual std::shared_ptr<Drawable> get(std::string const& name) const = 0;
  virtual bool remove(Shared<Drawable> const& r) = 0;
  virtual bool remove(std::string const& name) = 0;
  virtual void clear(std::optional<In> domain = std::nullopt) = 0;
  virtual void updateBackgroundImage(sophus::IntensityImage<> const& image) = 0;

  // Convenience method to add several drawables together
  template <typename... Ts>
  void bulkAddInScene(Ts const&... ts)
  {
    (add(DrawableConversionTraits<Ts>::makeDrawable(ts), In::scene), ...);
  }

  template <typename T>
  auto addInScene(const T& t)
  {
    auto d = DrawableConversionTraits<T>::makeDrawable(t);
    add(d, In::scene);
    return d;
  }

  template <typename T>
  auto addNamedInScene(std::string const& name, const T& r)
  {
    auto d = DrawableConversionTraits<T>::makeDrawable(r);
    add(d, In::scene, name);
    return d;
  }

  template <typename T>
  auto addInSceneAt(const T& t, sophus::Se3F64 const& world_from_drawable)
  {
    auto d = DrawableConversionTraits<T>::makeDrawable(t);
    d->pose.world_from_drawable = world_from_drawable;
    add(d, In::scene);
    return d;
  }

  template <typename T>
  auto addNamedInSceneAt(
      std::string const& name, const T& r,
      sophus::Se3F64 const& world_from_drawable)
  {
    auto d = DrawableConversionTraits<T>::makeDrawable(r);
    d->pose.world_from_drawable = world_from_drawable;
    add(d, In::scene, name);
    return d;
  }

  // Convenience method to add several drawables together
  template <typename... Ts>
  void bulkAddInPixels(Ts const&... ts)
  {
    (add(DrawableConversionTraits<Ts>::makeDrawable(ts), In::pixels), ...);
  }

  template <typename T>
  void addInPixels(const T& t)
  {
    add(DrawableConversionTraits<T>::makeDrawable(t), In::pixels);
  }

  template <typename T>
  void addNamedInPixels(std::string const& name, const T& r)
  {
    add(DrawableConversionTraits<T>::makeDrawable(r), In::pixels, name);
  }

  struct Params {
    std::string name = "";
    Size size_hint = {Parts{1}, Parts{1}};
    AspectPolicy aspect_policy = AspectPolicy::overdraw;
    ImageXy image_convention = Conventions::global().image_xy;
    ImageIndexing image_indexing = Conventions::global().image_indexing;

    Shared<DrawLayerHandler> handler = DrawLayerHandler::Create({});

    std::optional<sophus::CameraModel> camera = std::nullopt;
    std::optional<sophus::Se3F64> camera_from_world = std::nullopt;
    MinMax<double> near_far = {1e-3, 1e6};

    // Objects to draw through modelview transform
    std::vector<Shared<Drawable>> in_scene = {};

    // Objects to draw in camera-frame, including 2D image-plane drawing.
    // These are 'camera pixels', not 'screen pixels'.
    std::vector<Shared<Drawable>> in_pixels = {};
  };
  static Shared<DrawLayer> Create(Params p);
};

////////////////////////////////////////////////////////////////////////
// Traits for extending what types can be converted to drawables

template <DerivedFrom<Drawable> L>
struct DrawableConversionTraits<Shared<L>> {
  static Shared<Drawable> makeDrawable(Shared<L> const& x) { return x; }
};

template <DerivedFrom<Drawable> L>
struct DrawableConversionTraits<std::shared_ptr<L>> {
  static Shared<Drawable> makeDrawable(std::shared_ptr<L> const& x)
  {
    return x;
  }
};

template <typename T>
concept DrawableConvertable = requires(T x)
{
  {
    DrawableConversionTraits<T>::makeDrawable(x)
    } -> SameAs<Shared<Drawable>>;
};

template <DrawableConvertable T>
Shared<Drawable> makeDrawable(const T& v)
{
  return DrawableConversionTraits<T>::makeDrawable(v);
}

////////////////////////////////////////////////////////////////////////
// Direct conversion of anything drawable to a layer

// Helper for adding Drawables directly into layouts
template <DerivedFrom<Drawable> T>
struct LayerConversionTraits<Shared<T>> {
  static Shared<Layer> makeLayer(Shared<T> const& drawable)
  {
    return DrawLayer::Create({.in_scene = {makeDrawable(drawable)}});
  }
};

}  // namespace pangolin
