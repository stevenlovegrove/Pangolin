#include "camera_utils.h"
#include "gl_utils.h"

#include <pangolin/context/factory.h>
#include <pangolin/gl/glsl_program.h>
#include <pangolin/gl/uniform.h>
#include <pangolin/gui/drawn_primitives.h>
#include <pangolin/render/gl_vao.h>

namespace pangolin
{

constexpr GLenum toGlEnum(DrawnPrimitives::Type type)
{
  switch (type) {
    case DrawnPrimitives::Type::points:
      return GL_POINTS;
    case DrawnPrimitives::Type::lines:
      return GL_LINES;
    case DrawnPrimitives::Type::line_strip:
      return GL_LINE_STRIP;
    case DrawnPrimitives::Type::line_loop:
      return GL_LINE_LOOP;
    case DrawnPrimitives::Type::triangles:
      return GL_TRIANGLES;
    case DrawnPrimitives::Type::triangle_strip:
      return GL_TRIANGLE_STRIP;
    default:
      return 0;
  }
}

struct GlDrawnPrimitives : public DrawnPrimitives {
  void draw(const ViewParams& params) override
  {
    switch (element_type) {
      case DrawnPrimitives::Type::axes:
        drawAxes(params);
        break;
      case DrawnPrimitives::Type::shapes:
        drawShapes(params);
        break;
      case DrawnPrimitives::Type::path:
        drawPath(params);
        break;
      default:
        drawPointsLinesTriangles(params);
        break;
    }
  }

  Region3F64 boundsInParent() const override { return Region3F64::empty(); }

  void drawAxes(const ViewParams& params)
  {
    if (!prog) {
      prog = GlSlProgram::Create(
          {.sources = {
               {.origin = "/components/pango_opengl/shaders/main_axes.glsl"}}});
    }

    vertices->sync();
    if (!vertices->empty()) {
      auto bind_prog = prog->bind();
      auto bind_vao = vao.bind();

      u_intrinsics =
          (params.clip_from_image * params.image_from_camera).cast<float>();
      u_cam_from_drawable = params.camera_from_drawable.cast<float>();
      u_size = default_size;

      auto bind_bo = vertices->bind();
      PANGO_ENSURE(
          vertices->dataType() &&
          vertices->dataType()->is<sophus::Se3<float>>());
      // xyzw quaternion
      PANGO_GL(
          glVertexAttribPointer(0, 4, GL_FLOAT, false, 8 * sizeof(float), 0));
      // translation + uninitialized padding
      PANGO_GL(glVertexAttribPointer(
          1, 4, GL_FLOAT, false, 8 * sizeof(float),
          (uint8_t*)(4 * sizeof(float))));
      PANGO_GL(glEnableVertexAttribArray(0));
      PANGO_GL(glEnableVertexAttribArray(1));

      PANGO_GL(glPointSize(5.0));
      PANGO_GL(glDrawArrays(GL_POINTS, 0, vertices->size()));
    }
  }

  void drawShapes(const ViewParams& params)
  {
    constexpr int location_vertex = 0;
    constexpr int location_colors = 1;
    constexpr int location_shapes = 2;

    vertices->sync();
    colors->sync();
    shapes->sync();

    if (!prog) {
      GlSlProgram::Defines defines;
      defines["VERTEX_COLORS"] = std::to_string(!colors->empty());
      defines["VERTEX_SHAPES"] = std::to_string(!shapes->empty());

      prog = GlSlProgram::Create(
          {.sources =
               {{.origin =
                     "/components/pango_opengl/shaders/main_shapes.glsl"}},
           .program_defines = defines});
    }

    if (!vertices->empty()) {
      auto bind_prog = prog->bind();
      auto bind_vao = vao.bind();

      u_intrinsics =
          (params.clip_from_image * params.image_from_camera).cast<float>();
      u_cam_from_drawable = params.camera_from_drawable.cast<float>();
      if (size_in_pixels) {
        u_mode = 1;
        Eigen::Array2f size_pix = Eigen::Vector2f(default_size, default_size);
        u_size_clip = 2.0 * size_pix.array() /
                      params.viewport.range().cast<float>().array();
      } else {
        u_mode = 0;
        u_size = default_size;
      }

      vao.addVertexAttrib(location_vertex, *vertices);
      if (!colors->empty()) {
        vao.addVertexAttrib(location_colors, *colors);
      } else {
        u_color = default_color.cast<float>();
      }
      if (!shapes->empty()) {
        vao.addVertexAttrib(location_shapes, *shapes);
      } else {
        u_shape = static_cast<int>(default_shape);
      }

      PANGO_GL(glDrawArrays(GL_POINTS, 0, vertices->size()));
    }
  }

  void drawPath(const ViewParams& params)
  {
    constexpr int location_vertex = 0;
    constexpr int location_colors = 4;

    vertices->sync();
    colors->sync();

    if (!prog) {
      GlSlProgram::Defines defines;
      defines["VERTEX_COLORS"] = std::to_string(!colors->empty());

      prog = GlSlProgram::Create(
          {.sources =
               {{.origin = "/components/pango_opengl/shaders/"
                           "main_path.glsl"}},
           .program_defines = defines});
    }

    if (vertices->size() >= 3) {
      PANGO_ENSURE(vertices->dataType());
      auto bind_prog = prog->bind();
      auto bind_vao = vao.bind();

      u_intrinsics =
          (params.clip_from_image * params.image_from_camera).cast<float>();
      u_cam_from_drawable = params.camera_from_drawable.cast<float>();

      if (size_in_pixels) {
        u_mode = 1;
        Eigen::Array2f size_pix = Eigen::Vector2f(default_size, default_size);
        u_size_clip = 2.0 * size_pix.array() /
                      params.viewport.range().cast<float>().array();
      } else {
        u_mode = 0;
        u_size_clip = Eigen::Vector2f(default_size, default_size);
        u_size_clip = Eigen::Vector2f(1.0, 1.0);
        u_size = default_size;
      }

      if (!colors->empty()) {
        vao.addVertexAttrib(location_colors, *colors);
      } else if (geometry_texture->empty()) {
        u_color = default_color.cast<float>();
      }

      const sophus::RuntimePixelType data_type = *vertices->dataType();

      for (int i = 0; i < 4; ++i) {
        vao.addVertexAttrib(
            location_vertex + i, *vertices, i * data_type.bytesPerPixel());
      }

      PANGO_GL(glDrawArrays(GL_POINTS, 0, vertices->size() - 3));
    }
  }

  void drawPointsLinesTriangles(const ViewParams& params)
  {
    constexpr int location_vertex = 0;
    constexpr int location_colors = 1;
    constexpr int location_normals = 2;
    constexpr int location_uvs = 3;

    indices->sync();
    vertices->sync();
    colors->sync();
    normals->sync();
    uvs->sync();
    material_image->sync();
    geometry_texture->sync();

    if (!prog) {
      GlSlProgram::Defines defines;
      defines["VERTEX_COLORS"] = std::to_string(!colors->empty());
      defines["VERTEX_NORMALS"] = std::to_string(!normals->empty());
      defines["VERTEX_UVS"] = std::to_string(!uvs->empty());
      defines["USE_MATCAP"] = std::to_string(!material_image->empty());
      defines["USE_TEXTURE"] = std::to_string(!geometry_texture->empty());

      prog = GlSlProgram::Create(
          {.sources =
               {{.origin = "/components/pango_opengl/shaders/"
                           "main_primitives_points.glsl"}},
           .program_defines = defines});
    }

    if (!vertices->empty()) {
      auto bind_prog = prog->bind();
      auto bind_vao = vao.bind();

      u_intrinsics =
          (params.clip_from_image * params.image_from_camera).cast<float>();
      u_cam_from_drawable = params.camera_from_drawable.cast<float>();

      vao.addVertexAttrib(location_vertex, *vertices);
      if (!colors->empty()) {
        vao.addVertexAttrib(location_colors, *colors);
      } else if (geometry_texture->empty()) {
        u_color = default_color.cast<float>();
      }
      if (!normals->empty()) {
        vao.addVertexAttrib(location_normals, *normals);
      }
      if (!uvs->empty()) {
        vao.addVertexAttrib(location_uvs, *uvs);
      }
      std::optional<ScopedBind<DeviceTexture>> bind_matcap;
      if (!material_image->empty()) {
        PANGO_GL(glActiveTexture(GL_TEXTURE0));
        bind_matcap = material_image->bind();
      }
      std::optional<ScopedBind<DeviceTexture>> bind_texture;
      if (!geometry_texture->empty()) {
        PANGO_GL(glActiveTexture(GL_TEXTURE0));
        bind_texture = geometry_texture->bind();
      }

      PANGO_GL(glPointSize(default_size));

      if (indices->empty()) {
        PANGO_GL(glDrawArrays(toGlEnum(element_type), 0, vertices->size()));
      } else {
        PANGO_ENSURE(indices->dataType());
        const auto maybe_gl_fmt = glTypeInfo(*indices->dataType());
        const GlFormatInfo gl_fmt = SOPHUS_UNWRAP(maybe_gl_fmt);

        auto bind_ibo = indices->bind();
        PANGO_GL(glDrawElements(
            toGlEnum(element_type), indices->size(), gl_fmt.gl_type, 0));
      }
    }
  }

  private:
  std::shared_ptr<GlSlProgram> prog;

  GlVertexArrayObject vao = {};
  const GlUniform<Eigen::Matrix4f> u_intrinsics = {"proj"};
  const GlUniform<Eigen::Matrix4f> u_cam_from_drawable = {"cam_from_world"};
  const GlUniform<Eigen::Vector4f> u_color = {"color"};
  const GlUniform<uint> u_shape = {"shape"};
  const GlUniform<bool> u_mode = {"mode"};
  const GlUniform<float> u_size = {"size"};
  const GlUniform<Eigen::Vector2f> u_size_clip = {"size_clip"};
};

PANGO_CREATE(DrawnPrimitives)
{
  auto r = Shared<GlDrawnPrimitives>::make();
  r->element_type = p.element_type;
  r->default_color = p.default_color;
  r->default_size = p.default_size;
  r->default_shape = p.default_shape;
  r->size_in_pixels = p.size_in_pixels;
  return r;
}

}  // namespace pangolin
