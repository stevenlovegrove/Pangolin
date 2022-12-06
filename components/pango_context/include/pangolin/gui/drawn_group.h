#pragma once

#include <pangolin/gui/draw_layer.h>

namespace pangolin
{

struct DrawnGroup : public Drawable {

void draw(const ViewParams& params) override {
    // Assumption parent_from_drawable is already in params.camera_from_world
    // TODO: rename params.camera_from_world since it isn't the world but drawable frame.
    ViewParams child_params = params;

    for(auto& child : children) {
        child_params.camera_from_world = params.camera_from_world * child->parent_from_drawable;
        child->draw(child_params);
    }
}

MinMax<Eigen::Vector3d> boundsInParent() const override {
    auto bounds = MinMax<Eigen::Vector3d>::closed();
    for(const auto& child : children) {
        // TODO: need a frame transform here.
        bounds.extend(child->boundsInParent());
    }
    return bounds;
}
struct Params {
    std::vector<Shared<Drawable>> children;
    Eigen::Matrix4d parent_from_drawable;
};
static Shared<DrawnGroup> Create(const Params& p) {
    auto ret = Shared<DrawnGroup>::make();
    ret->children = p.children;
    ret->parent_from_drawable = p.parent_from_drawable;
    return ret;
}

std::vector<Shared<Drawable>> children;
};

}
