#pragma once

#include <sophus/sensor/orthographic.h>
#include <pangolin/maths/camera_look_at.h>
#include <pangolin/maths/projection.h>
#include <pangolin/utils/variant_overload.h>

namespace pangolin
{

inline double aspect(const sophus::ImageSize size) {
    return double(size.width) / double(size.height);
}

inline double aspect(const Eigen::Array2i size) {
    return double(size.x()) / double(size.y());
}

inline Eigen::Array2i toEigen(const sophus::ImageSize size)
{
    return {size.width, size.height};
}

inline sophus::ImageSize toImageSize(const Eigen::Array2i size)
{
    return {size[0], size[1]};
}

inline Eigen::Array2d axisScale(
    Eigen::Array2i viewport_dim,
    Eigen::Array2i object_dim
) {
    const double object_viewport_ratio = aspect(object_dim) / aspect(viewport_dim);
    return object_viewport_ratio > 1.0 ?
        Eigen::Array2d{1.0, 1.0/object_viewport_ratio} :
        Eigen::Array2d{object_viewport_ratio, 1.0};
}

inline Eigen::Matrix3d camera_from_image(const Eigen::Matrix4d& image_from_camera_4x4) {
    return invProjectionCameraFromImage(
        {image_from_camera_4x4(0,0), image_from_camera_4x4(1,1)},
        {image_from_camera_4x4(0,2), image_from_camera_4x4(1,2)}
    );
}

inline Eigen::Matrix4d transformImageFromCamera4x4(
    const sophus::CameraModel& cam
) {
    auto focal_distance_pixels = cam.focalLength().eval();
    auto principle_point = cam.principalPoint().eval();

    if(cam.distortionType() == sophus::CameraDistortionType::orthographic) {
        return (Eigen::Matrix4d() <<
            focal_distance_pixels[0], 0.0, 0.0, principle_point[0],
            0.0, focal_distance_pixels[1], 0.0, principle_point[1],
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0).finished();
    }else{
        return (Eigen::Matrix4d() <<
            focal_distance_pixels[0], 0.0, principle_point[0], 0.0,
            0.0, focal_distance_pixels[1], principle_point[1], 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0).finished();
    }

}

inline Eigen::Matrix3d linearCameraFromImage(const sophus::CameraModel& camera)
{
    return invProjectionCameraFromImage(camera.focalLength(), camera.principalPoint());
}


}
