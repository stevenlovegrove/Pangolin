#pragma once

#include <Eigen/Core>
#include <pangolin/render/device_texture.h>
#include <sophus2/calculus/region.h>
#include <sophus2/lie/similarity3.h>

namespace pangolin
{

struct ViewParams {
  sophus2::Region2I viewport = sophus2::Region2I::empty();
  sophus2::ImageSize camera_dim;
  sophus2::RegionF64 near_far = sophus2::RegionF64::empty();
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
  // and/or alternatives (Similarity3, Aff3, etc.) here when needed.
  sophus2::Similarity3F64 parent_from_drawable;
};

struct Drawable {
  virtual ~Drawable() {}
  virtual void draw(const ViewParams&) = 0;
  virtual sophus2::Region3F64 boundsInParent() const = 0;
  DrawablePose pose;
};

}  // namespace pangolin
