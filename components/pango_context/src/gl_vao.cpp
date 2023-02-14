#include <pangolin/gl/gl_type_info.h>
#include <pangolin/render/gl_vao.h>

namespace pangolin
{

template <>
ScopedBind<GlVertexArrayObject>::pScopedBind&
ScopedBind<GlVertexArrayObject>::getLocalActiveScopePtr()
{
  static thread_local pScopedBind x = nullptr;
  return x;
}

GlVertexArrayObject::GlVertexArrayObject() : vao(0)
{
  PANGO_ENSURE(glGenVertexArrays, "Bad GL version or GLEW not inited");
  PANGO_GL(glGenVertexArrays(1, &vao));
}

GlVertexArrayObject::~GlVertexArrayObject()
{
  if (vao) PANGO_GL(glDeleteVertexArrays(1, &vao));
}

ScopedBind<GlVertexArrayObject> GlVertexArrayObject::bind() const
{
  return {
      [v = this->vao]() { glBindVertexArray(v); },
      []() { glBindVertexArray(0); }};
}

bool isGlIntegralDatatype(GLenum datatype)
{
  switch (datatype) {
    case GL_FLOAT:
      [[fallthrough]];
    case GL_DOUBLE:
      [[fallthrough]];
    case GL_HALF_FLOAT:
      return false;
    default:
      return true;
  }
}

void GlVertexArrayObject::addVertexAttrib(
    GLuint attrib_location, const GlBuffer& bo, size_t offset_bytes,
    size_t stride_bytes, GLboolean normalized)
{
  auto vao_bind = bind();
  bo.Bind();

#ifdef __APPLE__
  PANGO_GL(glVertexAttribPointer(
      attrib_location, bo.count_per_element, bo.datatype, normalized,
      stride_bytes, (void*)offset_bytes));
#else
  if (isGlIntegralDatatype(bo.datatype)) {
    PANGO_GL(glVertexAttribIPointer(
        attrib_location, bo.count_per_element, bo.datatype, stride_bytes,
        (void*)offset_bytes));
  } else {
    PANGO_GL(glVertexAttribPointer(
        attrib_location, bo.count_per_element, bo.datatype, normalized,
        stride_bytes, (void*)offset_bytes));
  }
#endif

  PANGO_GL(glEnableVertexAttribArray(attrib_location));
  bo.Unbind();
}

void GlVertexArrayObject::addVertexAttrib(
    GLuint attrib_location, const DeviceBuffer& bo, size_t offset_bytes,
    size_t stride_bytes, GLboolean normalized)
{
  auto bind_vao = bind();
  auto bind_buf = bo.bind();

  PANGO_ENSURE(bo.dataType());
  const sophus::RuntimePixelType data_type = *bo.dataType();
  auto maybe_gl_fmt = glTypeInfo(data_type);
  const GlFormatInfo gl_fmt = SOPHUS_UNWRAP(maybe_gl_fmt);

  if (isGlIntegralDatatype(gl_fmt.gl_type)) {
    PANGO_GL(glVertexAttribIPointer(
        attrib_location, data_type.num_channels, gl_fmt.gl_type, stride_bytes,
        (void*)offset_bytes));
  } else {
    PANGO_GL(glVertexAttribPointer(
        attrib_location, data_type.num_channels, gl_fmt.gl_type, normalized,
        stride_bytes, (void*)offset_bytes));
  }
  PANGO_GL(glEnableVertexAttribArray(attrib_location));
}

}  // namespace pangolin
