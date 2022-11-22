#pragma once

#include <sophus/sensor/orthographic.h>
#include <pangolin/maths/camera_look_at.h>
#include <pangolin/maths/projection.h>
#include <pangolin/utils/variant_overload.h>

namespace pangolin
{

inline Eigen::Array2i toEigen(const sophus::ImageSize size)
{
    return {size.width, size.height};
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
