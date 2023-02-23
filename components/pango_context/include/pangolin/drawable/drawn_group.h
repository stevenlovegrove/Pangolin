#pragma once

#include <pangolin/layer/draw_layer.h>

#include <filesystem>

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

  sophus::Region3F64 boundsInParent() const override
  {
    auto bounds = sophus::Region3F64::empty();
    for (const auto& child : children) {
      // TODO: need a frame transform here.
      bounds.extend(child->boundsInParent());
    }
    return bounds;
  }
  struct Params {
    std::optional<std::filesystem::path> file_assets;
    std::vector<Shared<Drawable>> children;
  };
  static Shared<DrawnGroup> Create(const Params& p);

  std::vector<Shared<Drawable>> children;
};

}  // namespace pangolin
