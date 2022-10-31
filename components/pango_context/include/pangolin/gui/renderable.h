#pragma once

#include <vector>
#include <variant>

#include <sophus/image/runtime_image.h>
#include <pangolin/maths/conventions.h>
#include <pangolin/render/device_buffer.h>

namespace pangolin
{

struct RenderableImage
{
    enum class Interpolation
    {
        nearest,
        bilinear,
    };

    // Image to render. Not all types will
    // be supported by the implementation.
    Shared<DeviceBuffer> image =
        DeviceBuffer::Create({DeviceBuffer::Kind::Texture});

    // How should fractional pixel coordinates be
    // rendered (when magnified)
    Interpolation interpolation = Interpolation::nearest;

    // optional transform which maps the pixel color
    // space to the rendered output intensity
    std::optional<Eigen::MatrixXd> color_transform;

    // The 'image' frame is such that the pixels
    // lie on the z=0 plane, with the image x and y
    // axis corresponding to the world co-ordinates
    // in -continuous- convention.
    // i.e. the the x=0,y=0,z=0 frame point would
    // be (-0.5,-0.5) in pixel centered integral
    // coordinate convention.
    std::optional<Eigen::Matrix4d> world_from_image;
};

struct RenderablePts
{
    enum class Type
    {
        // Sized in image pixels
        points,
        // Sized in world coordinates and oriented
        circles,
        squares,
        // lines and triangles
        lines,
        line_strip,
        triangles,
        triangle_strip
    };

    // Vertex data to render
    Shared<DeviceBuffer> vertices;

    // If provided, use as index buffer
    Shared<DeviceBuffer> indices;

    // If provided, use per-vertex colors
    Shared<DeviceBuffer> colors;

    // If provided, use per-vertex normals
    Shared<DeviceBuffer> normals;

    // If provided, use per_vertex radius for oriented disks
    Shared<DeviceBuffer> radius;

    // Geometric element to interpret vertices as
    Type point_type = Type::points;

    // Transform to apply to points during render
    // (they will be further transformed by the camera
    //  and modelview style matrices)
    Eigen::Matrix4d world_from_renderable;

    // Color to use if colors buffer is empty
    Eigen::Vector4f default_color = {1.0f, 1.0f, 1.0f, 1.0f};

    // Element size to use for points/circles/squares if
    // radius buffer is empty. points are in pixel
    // units. Other elements are world units.
    float default_radius = 1.0;
};

using Renderable = std::variant<RenderableImage,RenderablePts>;

}
