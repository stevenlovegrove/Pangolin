#include <pangolin/gl/glvao.h>

namespace pangolin
{

template<>
thread_local ScopedBind<GlVertexArrayObject>* ScopedBind<GlVertexArrayObject>::current = nullptr;


GlVertexArrayObject::GlVertexArrayObject()
    : vao(0)
{
    PANGO_ENSURE(glGenVertexArrays, "Bad GL version or GLEW not inited");
    PANGO_GL(glGenVertexArrays(1, &vao));
}

GlVertexArrayObject::~GlVertexArrayObject()
{
    if(vao) PANGO_GL(glDeleteVertexArrays(1, &vao));
}

ScopedBind<GlVertexArrayObject> GlVertexArrayObject::bind() const
{
    return {
        [v=this->vao](){glBindVertexArray(v);},
        [](){glBindVertexArray(0);}
    };
}

bool isGlIntegralDatatype(GLenum datatype) {
    switch (datatype) {
    case GL_FLOAT:  [[fallthrough]];
    case GL_DOUBLE: [[fallthrough]];
    case GL_HALF_FLOAT:
        return false;
    default:
        return true;
    }
}

void GlVertexArrayObject::addVertexAttrib(GLuint attrib_location, const GlBuffer& bo, size_t offset_bytes, size_t stride_bytes, GLboolean normalized)
{
    auto vao_bind = bind();
    bo.Bind();

    // if(PANGO_GL(isGlIntegralDatatype(bo.datatype))) {
    //     PANGO_GL(glVertexAttribIPointer(attrib_location, bo.count_per_element, bo.datatype, stride_bytes, (void*)offset_bytes ));
    // }else{
        PANGO_GL(glVertexAttribPointer(attrib_location, bo.count_per_element, bo.datatype, normalized, stride_bytes, (void*)offset_bytes));
    // }
    PANGO_GL(glEnableVertexAttribArray(attrib_location));
}

}
