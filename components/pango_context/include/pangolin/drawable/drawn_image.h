#pragma once

#include <pangolin/color/colormap.h>
#include <pangolin/drawable/drawn_checker.h>
#include <pangolin/layer/draw_layer.h>
#include <pangolin/render/conventions.h>
#include <pangolin/render/device_texture.h>
#include <sophus2/image/dyn_image_types.h>

namespace pangolin
{

// The 'image' frame is such that the pixels lie on the z=0 plane, with the
// image x and y axis corresponding to the world co-ordinates in -continuous-
// convention. i.e. the the x=0,y=0,z=0 frame point would be (-0.5,-0.5) in
// pixel centered integral coordinate convention.
struct DrawnImage : public Drawable {
  enum class Interpolation {
    nearest,
    bilinear,
  };

  // Image to render. Not all types will be supported by the implementation.
  Shared<DeviceTexture> image = DeviceTexture::Create();

  // How should fractional pixel coordinates be rendered (when magnified)
  Interpolation interpolation = Interpolation::nearest;

  // optional transform which maps the pixel color space to the rendered
  // output intensity
  std::optional<Eigen::Matrix4d> color_transform;

  // if a palatte beside `none` is specified, the first (red) channel is used
  // as input to the non-linear map. Colormapping occurs after the linear
  // color_transform above.
  Palette colormap = Palette::none;

  struct Params {
    sophus2::IntensityImage<> image = {};
    Palette colormap = Palette::none;
    Interpolation interpolation = Interpolation::nearest;
    std::optional<Eigen::Matrix4d> color_transform = std::nullopt;
  };
  static Shared<DrawnImage> Create(Params p);
};

////////////////////////////////////////////////////////////////////

template <typename T>
concept ImageConvertable = requires(T x)
{
  {
    sophus2::IntensityImage<>(x)
    } -> SameAs<sophus2::IntensityImage<>>;
};

template <ImageConvertable T>
sophus2::CameraModel defaultOrthoCameraForImage(const T& image)
{
  return sophus2::CameraModel(
      image.imageSize(), sophus2::CameraDistortionType::orthographic,
      Eigen::Vector4d{1.0, 1.0, 0.0, 0.0});
}

template <ImageConvertable T>
struct DrawableConversionTraits<T> {
  static Shared<Drawable> makeDrawable(const T& image)
  {
    return DrawnImage::Create({.image = image});
  }
};

// Helper for adding images (runtime and statically typed) directly to layouts
template <ImageConvertable T>
struct LayerConversionTraits<T> {
  static Shared<DrawLayer> makeLayer(const T& image)
  {
    return DrawLayer::Create(
        {.in_pixels = {
             DrawnChecker::Create({}), DrawnImage::Create({.image = image})}});
  }
};

// Specialization of draw_layer.h's trait for DrawnImage so it defaults to
// add to .in_pixels instead of .in_scene
template <>
struct LayerConversionTraits<Shared<DrawnImage>> {
  static Shared<DrawLayer> makeLayer(const Shared<DrawnImage>& drawable)
  {
    return DrawLayer::Create(
        {.in_pixels = {DrawnChecker::Create({}), drawable}});
  }
};

}  // namespace pangolin
