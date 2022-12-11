#pragma once

#include <Eigen/Core>
#include <pangolin/render/device_texture.h>

namespace pangolin
{

struct ViewParams {
    MinMax<Eigen::Array2i> viewport;
    sophus::ImageSize camera_dim;
    MinMax<double> near_far;
    Eigen::Matrix4d camera_from_world;
    Eigen::Matrix4d image_from_camera;
    Eigen::Matrix4d clip_from_image;
    std::shared_ptr<DeviceTexture> unproject_map;
};

struct Drawable {
    virtual ~Drawable() {}
    virtual void draw(const ViewParams&) = 0;
    virtual MinMax<Eigen::Vector3d> boundsInParent() const = 0;
    Eigen::Matrix4d parent_from_drawable = Eigen::Matrix4d::Identity();
};


}
