#pragma once

#include <Eigen/Core>
#include <pangolin/maths/min_max.h>
#include <pangolin/render/device_texture.h>

namespace pangolin
{

struct ViewParams {
  Interval<Eigen::Array2i> viewport;
  sophus::ImageSize camera_dim;
  Interval<double> near_far;
  Eigen::Matrix4d camera_from_drawable;
  Eigen::Matrix4d image_from_camera;
  Eigen::Matrix4d clip_from_image;
  std::shared_ptr<DeviceTexture> unproject_map;
};

struct DrawablePose {
  // Here the 'parent' frame is either the world frame, or a parent drawable.
  Eigen::Matrix4d parentFromDrawableMatrix()
  {
    return parent_from_drawable.matrix();
  }

  // Add Eigen::Matrix4d representation augmentations (e.g. drawable_scales),
  // and/or alternatives (Sim3, Aff3, etc.) here when needed.
  sophus::Se3F64 parent_from_drawable;
};

struct Drawable {
  virtual ~Drawable() {}
  virtual void draw(const ViewParams&) = 0;
  virtual Interval<Eigen::Vector3d> boundsInParent() const = 0;
  DrawablePose pose;
};

}  // namespace pangolin
