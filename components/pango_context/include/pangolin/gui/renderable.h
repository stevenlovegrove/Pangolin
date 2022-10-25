#pragma once

#include <vector>
#include <variant>

#include <sophus/image/runtime_image.h>
#include <pangolin/maths/conventions.h>

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
    sophus::AnyImage<> image;

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

    using vecid = std::vector<uint32_t>;
    using vec1 = std::vector<float>;
    using vec2 = std::vector<Eigen::Vector2f>;
    using vec3 = std::vector<Eigen::Vector3f>;
    using vec4 = std::vector<Eigen::Vector4f>;

    // Vertex data to render
    std::variant<vec2,vec3,vec4> vertices;

    // If provided, use as index buffer
    std::variant<std::monostate,vecid> indices;

    // If provided, use per-vertex colors
    std::variant<std::monostate,vec3,vec4> colors;

    // If provided, use per-vertex normals
    std::variant<std::monostate,vec3> normals;

    // If provided, use per_vertex radius for oriented disks
    std::variant<std::monostate,vec1> radius;

    // Geometric element to interpret vertices as
    Type point_type = Type::points;

    // Transform to apply to points during render
    // (they will be further transformed by the camera
    //  and modelview style matrices)
    Eigen::Matrix4d world_from_renderable;

    // Color to use if colors variant not provided
    Eigen::Vector4f default_color = {1.0f, 1.0f, 1.0f, 1.0f};

    // Element size to use for points/circles/squares if
    // radius variant is not provided. points are in pixel
    // units. Other elements are world units.
    float default_radius = 1.0;
};

using Renderable = std::variant<RenderableImage,RenderablePts>;

}
