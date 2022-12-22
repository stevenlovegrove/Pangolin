#pragma once

#include <pangolin/gui/draw_layer.h>

namespace pangolin
{

struct DrawnGroup : public Drawable {
  void draw(const ViewParams& params) override
  {
    ViewParams child_params = params;

    for (auto& child : children) {
      child_params.camera_from_drawable =
          params.camera_from_drawable * child->pose.parentFromDrawableMatrix();
      child->draw(child_params);
    }
  }

  MinMax<Eigen::Vector3d> boundsInParent() const override
  {
    auto bounds = MinMax<Eigen::Vector3d>::closed();
    for (const auto& child : children) {
      // TODO: need a frame transform here.
      bounds.extend(child->boundsInParent());
    }
    return bounds;
  }
  struct Params {
    std::vector<Shared<Drawable>> children;
  };
  static Shared<DrawnGroup> Create(const Params& p)
  {
    auto ret = Shared<DrawnGroup>::make();
    ret->children = p.children;
    return ret;
  }

  std::vector<Shared<Drawable>> children;
};

}  // namespace pangolin
