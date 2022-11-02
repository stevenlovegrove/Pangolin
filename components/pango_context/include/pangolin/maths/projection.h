#pragma once

#include <pangolin/maths/conventions.h>
#include <pangolin/maths/min_max.h>
#include <sophus/image/image_size.h>

namespace pangolin
{

Eigen::Matrix<double,4,4> projectionClipFromCamera(
    sophus::ImageSize size,
    double focal_distance_pixels,
    Eigen::Vector2d principle_point,
    MinMax<double> near_far_in_world_units,
    DeviceXyz coord_convention = Conventions::global().device_xyz,
    ImageXy image_convention = Conventions::global().image_xy,
    ImageIndexing image_indexing = Conventions::global().image_indexing
);

Eigen::Matrix<double,4,4> projectionClipFromOrtho(
    MinMax<Eigen::Vector2d> extent,
    MinMax<double> near_far_in_world_units,
    ImageXy image_convention = Conventions::global().image_xy,
    ImageIndexing image_indexing = Conventions::global().image_indexing
);

}
