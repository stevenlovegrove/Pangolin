#pragma once

#include <pangolin/gl/gl.h>
#include <pangolin/gl/scoped_bind.h>
#include <pangolin/render/device_buffer.h>
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

    void addVertexAttrib(
        GLuint attrib_location,
        const DeviceBuffer& bo,
        size_t offset_bytes = 0,
        size_t stride_bytes = 0,
        GLboolean normalized = GL_FALSE
    );


protected:
    GLuint vao;
};

}
