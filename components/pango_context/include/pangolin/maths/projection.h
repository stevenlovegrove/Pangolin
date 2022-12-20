#pragma once

#include <pangolin/maths/conventions.h>
#include <pangolin/maths/min_max.h>
#include <sophus/image/image_size.h>

namespace pangolin
{

// Summary of intermediate coordinate frames we may refer to, showing how points
// transform from world coordinates through the rendering pipeline
//
// Window <- NDC <- Clip <- Projection (u,v,zbuffer) <- Image (u,v,depth) <- Camera (x,y,z) <- World

// Window: Pixel coordinates relative to the application window
// NDC: Normalized Device Coordinates (x,y,z) \in R^3. (x,y,z) \in [-1, 1]

Eigen::Matrix3d projectionImageFromCamera(
    Eigen::Vector2d focal_distance_pixels,
    Eigen::Vector2d principle_point
);

Eigen::Matrix3d invProjectionCameraFromImage(
    Eigen::Vector2d focal_distance_pixels,
    Eigen::Vector2d principle_point
);

// Returns a 4x4 matrix which acts to transform a homegenous 4-point such that
// it is ready for graphics hardware w division and with remapped NDC depth.
// Notably, this matrix does not map x,y into clip coordinates. It is
// recommended to set 'far' to infinity for perspective projections, and a
// finite value for orthographic.
Eigen::Matrix4d transformProjectionFromImage(
    MinMax<double> near_far_in_world_units,
    GraphicsProjection projection = GraphicsProjection::perspective
);

// Returns 4x4 transform which takes z-buffer mapped pixel homogenious points
// into clip coordinates (with interval [-1, -1, -1] to [1,1,1]).
Eigen::Matrix4d transformClipFromProjection(
    sophus::ImageSize size,
    ImageXy image_convention = Conventions::global().image_xy,
    ImageIndexing image_indexing = Conventions::global().image_indexing
);

Eigen::Matrix3d transformWindowFromClip(MinMax<Eigen::Array2i> viewport);


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
