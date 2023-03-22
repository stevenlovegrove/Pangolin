#include <pangolin/drawable/drawn_primitives.h>
#include <pangolin/gl/gl_scoped_enable.h>
#include <pangolin/gl/gl_vao.h>
#include <pangolin/gl/glsl_program.h>
#include <pangolin/gl/uniform.h>
#include <pangolin/utils/shared.h>

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

  sophus::Region3F64 boundsInParent() const override
  {
    return sophus::Region3F64::empty();
  }

  void drawAxes(const ViewParams& params)
  {
    if (prog_kind != ProgramKind::axis) {
      prog_kind = ProgramKind::axis;
      prog->reload(
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
          vertices->dataType()->is<sophus::Isometry3F32>());
      // xyzw quaternion
      PANGO_GL(
          glVertexAttribPointer(0, 4, GL_FLOAT, false, 7 * sizeof(float), 0));
      // translation + uninitialized padding
      PANGO_GL(glVertexAttribPointer(
          1, 3, GL_FLOAT, false, 7 * sizeof(float),
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

    if (prog_kind != ProgramKind::shapes) {
      prog_kind = ProgramKind::shapes;

      GlSlProgram::Defines defines;
      defines["VERTEX_COLORS"] = std::to_string(!colors->empty());
      defines["VERTEX_SHAPES"] = std::to_string(!shapes->empty());

      prog->reload(
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

    if (prog_kind != ProgramKind::path) {
      prog_kind = ProgramKind::path;

      GlSlProgram::Defines defines;
      defines["VERTEX_COLORS"] = std::to_string(!colors->empty());

      prog->reload(
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

      ScopedGlDisable disable_depth(depth_test ? 0 : GL_DEPTH_TEST);

      const sophus::PixelFormat data_type = *vertices->dataType();

      for (int i = 0; i < 4; ++i) {
        vao.addVertexAttrib(
            location_vertex + i, *vertices, i * data_type.numBytesPerPixel());
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

    if (prog_kind != ProgramKind::primitives) {
      prog_kind = ProgramKind::primitives;

      GlSlProgram::Defines defines;
      defines["VERTEX_COLORS"] = std::to_string(!colors->empty());
      defines["VERTEX_NORMALS"] = std::to_string(!normals->empty());
      defines["VERTEX_UVS"] = std::to_string(!uvs->empty());
      defines["USE_MATCAP"] = std::to_string(!material_image->empty());
      defines["USE_TEXTURE"] = std::to_string(!geometry_texture->empty());

      prog->reload(
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
      } else if (normals->empty() && material_image->empty()) {
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

      ScopedGlDisable disable_depth(depth_test ? 0 : GL_DEPTH_TEST);

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
  enum class ProgramKind { none, axis, shapes, path, primitives };
  ProgramKind prog_kind = ProgramKind::none;
  Shared<GlSlProgram> prog = GlSlProgram::Create({});

  GlVertexArrayObject vao = {};
  GlUniform<Eigen::Matrix4f> u_intrinsics = {prog, "proj"};
  GlUniform<Eigen::Matrix4f> u_cam_from_drawable = {prog, "cam_from_world"};
  GlUniform<Eigen::Vector4f> u_color = {prog, "color"};
  GlUniform<uint> u_shape = {prog, "shape"};
  GlUniform<bool> u_mode = {prog, "mode"};
  GlUniform<float> u_size = {prog, "size"};
  GlUniform<Eigen::Vector2f> u_size_clip = {prog, "size_clip"};
};

Shared<DrawnPrimitives> DrawnPrimitives::Create(DrawnPrimitives::Params p)
{
  auto r = Shared<GlDrawnPrimitives>::make();
  r->element_type = p.element_type;
  r->default_color = p.default_color;
  r->default_size = p.default_size;
  r->default_shape = p.default_shape;
  r->size_in_pixels = p.size_in_pixels;
  r->depth_test = p.depth_test;
  return r;
}

}  // namespace pangolin
