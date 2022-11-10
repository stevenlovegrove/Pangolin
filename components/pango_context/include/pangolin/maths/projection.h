#pragma once

#include <pangolin/maths/conventions.h>
#include <pangolin/maths/min_max.h>
#include <sophus/image/image_size.h>

namespace pangolin
{

Eigen::Matrix3d projectionImageFromCamera(
    Eigen::Vector2d focal_distance_pixels,
    Eigen::Vector2d principle_point
);

Eigen::Matrix3d invProjectionCameraFromImage(
    Eigen::Vector2d focal_distance_pixels,
    Eigen::Vector2d principle_point
);

Eigen::Matrix4d projectionClipFromCamera(
    sophus::ImageSize size,
    Eigen::Vector2d focal_distance_pixels,
    Eigen::Vector2d principle_point,
    MinMax<double> near_far_in_world_units,
    DeviceXyz coord_convention = Conventions::global().device_xyz,
    ImageXy image_convention = Conventions::global().image_xy,
    ImageIndexing image_indexing = Conventions::global().image_indexing
);

Eigen::Matrix4d projectionClipFromOrtho(
    MinMax<Eigen::Vector2d> extent,
    MinMax<double> near_far_in_world_units,
    ImageXy image_convention = Conventions::global().image_xy,
    ImageIndexing image_indexing = Conventions::global().image_indexing
);

}
