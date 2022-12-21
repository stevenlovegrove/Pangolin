#pragma once

#include <pangolin/gui/draw_layer.h>

namespace pangolin
{

// By design, ignoring the pose of the children
struct DrawnGroup : public Drawable {
  void draw(const ViewParams& params) override
  {
    for (auto& child : children) {
      child->draw(params);
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
