#pragma once

#include <pangolin/gui/draw_layer.h>
#include <pangolin/maths/conventions.h>
#include <pangolin/render/colormap.h>
#include <pangolin/render/device_buffer.h>
#include <pangolin/render/device_texture.h>
#include <sophus/image/runtime_image_types.h>

#include <vector>

namespace pangolin
{

struct DrawnPrimitives : public Drawable {
  enum class Type {
    // Sized in image pixels
    points,  // populate vertices, [indices,colors,radius]
    shapes,
    axes,  // populate vertices with axes_T_world sophus::SE3f elements
           // [indices,colors,radius]
           // lines and triangles
    path,  // populate vertices with centerline of path
    lines,
    line_strip,
    line_loop,
    triangles,
    triangle_strip
  };

  enum class Shape {
    filled_circle = 0,
    filled_box,
    filled_rhombus,
    filled_equilateraltriangle,
    filled_pentagon,
    filled_hexagon,
    filled_hexagram,
    filled_star,
    filled_heart,
    filled_roundedx,
    filled_blobbycross,
    hollow_circle,
    hollow_box,
    hollow_rhombus,
    hollow_equilateraltriangle,
    hollow_pentagon,
    hollow_hexagon,
    hollow_hexagram,
    hollow_star,
    hollow_heart,
    hollow_roundedx,
    hollow_blobbycross
  };

  // Vertex data to render
  Shared<DeviceBuffer> vertices =
      DeviceBuffer::Create({.kind = DeviceBuffer::Kind::VertexAttributes});

  // If provided, use as index buffer
  Shared<DeviceBuffer> indices =
      DeviceBuffer::Create({.kind = DeviceBuffer::Kind::VertexIndices});

  // If provided, use per-vertex colors
  Shared<DeviceBuffer> colors =
      DeviceBuffer::Create({.kind = DeviceBuffer::Kind::VertexAttributes});

  // If provided, use per-vertex normals
  Shared<DeviceBuffer> normals =
      DeviceBuffer::Create({.kind = DeviceBuffer::Kind::VertexAttributes});

  // If provided, use per-vertex normals
  Shared<DeviceBuffer> uvs =
      DeviceBuffer::Create({.kind = DeviceBuffer::Kind::VertexAttributes});

  // If provided, use per_vertex radius for oriented disks
  Shared<DeviceBuffer> radius =
      DeviceBuffer::Create({.kind = DeviceBuffer::Kind::VertexAttributes});

  // If provided, use per_vertex integral shape
  Shared<DeviceBuffer> shapes =
      DeviceBuffer::Create({.kind = DeviceBuffer::Kind::VertexAttributes});

  // If this and normals are provided, will use material_image as
  // a 'matcap' texture, providing a lookup from normal to color.
  Shared<DeviceTexture> material_image = DeviceTexture::Create({});

  // Texture to use if uv's are specified for triangle primitives
  Shared<DeviceTexture> geometry_texture = DeviceTexture::Create({});

  // Geometric element to interpret vertices as
  Type element_type;

  // Color to use if colors buffer is empty
  Eigen::Vector4d default_color;

  // Element size to use for points/circles/squares if radius buffer is empty.
  // points are in pixel units. Other elements are world units.
  double default_size;

  // To use when `shapes` is empty
  Shape default_shape;

  struct Params {
    Type element_type = Type::points;
    Eigen::Vector4d default_color = {1.0f, 0.0f, 0.0f, 1.0f};
    double default_size = 1.0;
    Shape default_shape = Shape::filled_circle;
  };
  static Shared<DrawnPrimitives> Create(Params p);
};

}  // namespace pangolin
