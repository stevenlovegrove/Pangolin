#pragma once

#include <pangolin/gui/draw_layer.h>
#include <pangolin/maths/conventions.h>
#include <pangolin/render/colormap.h>
#include <pangolin/render/device_texture.h>
#include <sophus/image/runtime_image_types.h>
#include <sophus/sensor/orthographic.h>

namespace pangolin
{

// Renders a graph-paper-like backdrop for plotting on-top of.
struct DrawnPlotBackground : public Drawable {
  struct Params {
    // Plot background color
    Eigen::Vector4f color_background = {0.95, 0.95, 0.95, 1.0};

    // The ammount to scale the background color for each overlapping tick
    // > 1.0 will lighten the graph
    // < 1.0 will darken the graph
    Eigen::Vector4f tick_color_scale = {0.90, 0.90, 0.90, 1.0};

    // The scale from one tick octave to the next
    float num_divisions = 5.0;
  };
  static Shared<DrawnPlotBackground> Create(Params p);
};

/// Convenience utility for defining an appropriate camera for use with
/// plotting on a DrawLayer.
/// Returns orthographic camera model given bounding box and image size.
template <class TScalar>
sophus::OrthographicModelT<TScalar> plot_camera(
    const Eigen::AlignedBox<TScalar, 2>& bounding_box)
{
  // Plotter is not discretized into pixels, so we use a unit width and
  // height

  const Eigen::Array<TScalar, 2, 1> range =
      bounding_box.max() - bounding_box.min();
  const Eigen::Array<TScalar, 2, 1> scale =
      Eigen::Array<TScalar, 2, 1>(1.0, 1.0) / range;
  const Eigen::Array<TScalar, 2, 1> offset =
      -(scale * bounding_box.min().array());

  const Eigen::Matrix<TScalar, 4, 1> params(
      scale.x(), scale.y(), offset.x(), offset.y());

  return sophus::OrthographicModelT<TScalar>({1, 1}, params);
}

}  // namespace pangolin
