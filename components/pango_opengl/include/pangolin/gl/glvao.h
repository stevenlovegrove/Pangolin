#pragma once

#include <pangolin/gl/gl.h>
#include <pangolin/gl/scoped_bind.h>
#include <vector>

namespace pangolin
{

class PANGOLIN_EXPORT GlVertexArrayObject
{
public:
    GlVertexArrayObject();
    ~GlVertexArrayObject();

    ScopedBind<GlVertexArrayObject> bind() const;

    void addVertexAttrib(
        GLuint attrib_location,
        const GlBuffer& bo,
        size_t offset_bytes = 0,
        size_t stride_bytes = 0,
        GLboolean normalized = GL_FALSE
    );

protected:
    GLuint vao;

    // we'll only use this on platforms that don't support VAO's
    // in order to mimic it's functionality
    struct Attrib
    {
        Attrib(GLuint attrib_location, const GlBuffer& bo, size_t offset_bytes, size_t stride_bytes, GLboolean normalized)
        : attrib_location(attrib_location), bo(bo), offset_bytes(offset_bytes),
          stride_bytes(stride_bytes), normalized(normalized)
        {}

        GLuint attrib_location;
        const GlBuffer& bo;
        size_t offset_bytes;
        size_t stride_bytes;
        GLboolean normalized;
    };

    void enableAttrib(const Attrib& attr) const;

    void disableAttrib(const Attrib& attr) const;

    std::vector<Attrib> attribs;
};

}
