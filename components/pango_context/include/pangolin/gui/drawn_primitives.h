#pragma once

#include <vector>

#include <sophus/image/runtime_image.h>
#include <pangolin/maths/conventions.h>
#include <pangolin/render/device_buffer.h>
#include <pangolin/render/device_texture.h>
#include <pangolin/render/colormap.h>

#include <pangolin/gui/draw_layer.h>

namespace pangolin
{

struct DrawnPrimitives : public DrawLayer::Drawable
{
    enum class Type
    {
        // Sized in image pixels
        points,   // populate vertices, [indices,colors,radius]
        // Sized in world coordinates and oriented
        circles,  // populate vertices, [indices,colors,radius]
        squares,  // populate vertices, [indices,colors,radius]
        axes,     // populate vertices with axes_T_world sophus::SE3f elements
                  // [indices,colors,radius]
        // lines and triangles
        lines,
        line_strip,
        line_loop,
        triangles,
        triangle_strip
    };

    // Vertex data to render
    Shared<DeviceBuffer> vertices = DeviceBuffer::Create({
        .kind=DeviceBuffer::Kind::VertexAttributes
    });

    // If provided, use as index buffer
    Shared<DeviceBuffer> indices = DeviceBuffer::Create({
        .kind=DeviceBuffer::Kind::VertexIndices
    });

    // If provided, use per-vertex colors
    Shared<DeviceBuffer> colors = DeviceBuffer::Create({
        .kind=DeviceBuffer::Kind::VertexAttributes
    });

    // If provided, use per-vertex normals
    Shared<DeviceBuffer> normals = DeviceBuffer::Create({
        .kind=DeviceBuffer::Kind::VertexAttributes
    });

    // If provided, use per_vertex radius for oriented disks
    Shared<DeviceBuffer> radius = DeviceBuffer::Create({
        .kind=DeviceBuffer::Kind::VertexAttributes
    });

    // Geometric element to interpret vertices as
    Type element_type;

    // Color to use if colors buffer is empty
    Eigen::Vector4d default_color;

    // Element size to use for points/circles/squares if radius buffer is empty.
    // points are in pixel units. Other elements are world units.
    double default_radius;

    struct Params {
        Type element_type = Type::points;
        Eigen::Vector4d default_color = {1.0f, 0.0f, 0.0f, 1.0f};
        double default_radius = 1.0;
        Eigen::Matrix4d parent_from_drawable = Eigen::Matrix4d::Identity();
    };
    static Shared<DrawnPrimitives> Create(Params p);
};

}
