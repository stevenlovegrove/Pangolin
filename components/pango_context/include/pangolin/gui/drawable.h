#pragma once

#include <Eigen/Core>
#include <pangolin/maths/region.h>
#include <pangolin/render/device_texture.h>

namespace pangolin
{

struct ViewParams {
  Region2I viewport = Region2I::empty();
  sophus::ImageSize camera_dim;
  RegionF64 near_far = RegionF64::empty();
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
  virtual Region3F64 boundsInParent() const = 0;
  DrawablePose pose;
};

}  // namespace pangolin
