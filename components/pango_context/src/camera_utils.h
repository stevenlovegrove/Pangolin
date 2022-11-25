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
    // This is the non-projective bit which is the same for
    // perspective and orthographic cameras.
    return transformImageFromCamera4x4( projectionImageFromCamera(
        cam.focalLength(), cam.principalPoint()
    ));
}

inline Eigen::Matrix4d linearClipFromCamera(const sophus::CameraModel& camera, MinMax<double> near_far)
{
    using namespace sophus;

    return std::visit(overload{
        [&](const OrthographicModel& cam){
            const auto bb = boundingBoxFromOrthoCam(cam);
            const MinMax<Eigen::Vector2d> extent(bb.min(),bb.max());

            return projectionClipFromOrtho(
                extent, near_far,
                ImageXy::right_down,
                // already specified correctly in OrthographicModel's coords
                ImageIndexing::pixel_continuous
            );
        },
        [&](const auto& cam){
            if(camera.distortionType() != sophus::CameraDistortionType::pinhole) {
                PANGO_WARN("Ignoring distortion component of camera for OpenGL rendering for now.");
            }
            return projectionClipFromCamera(
                cam.imageSize(), cam.focalLength(),
                cam.principalPoint(), near_far
            );
        }
    }, camera.modelVariant());
}

inline Eigen::Matrix3d linearCameraFromImage(const sophus::CameraModel& camera)
{
    return invProjectionCameraFromImage(camera.focalLength(), camera.principalPoint());
}


}
