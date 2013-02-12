/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011 Steven Lovegrove
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef PANGOLIN_GL_H
#define PANGOLIN_GL_H

#include "platform.h"

#ifdef _WIN_
#include <Windows.h>
#endif

#include <GL/glew.h>

#ifdef _OSX_
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <math.h>
#include <iostream>
#include <cstdlib>

namespace pangolin
{

#define CheckGlDieOnError() pangolin::_CheckGlDieOnError( __FILE__, __LINE__ );

////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////

class GlTexture
{
public:
  //! internal_format normally one of GL_RGBA8, GL_LUMINANCE8, GL_INTENSITY16
  GlTexture(GLint width, GLint height, GLint internal_format = GL_RGBA8, bool sampling_linear = true, int border = 0, GLenum glformat = GL_RGBA, GLenum gltype = GL_UNSIGNED_BYTE );

#if __cplusplus > 199711L
  //! Move Constructor
  GlTexture(GlTexture&& tex);
#endif

  //! Default constructor represents 'no texture'
  GlTexture();
  ~GlTexture();

  //! Reinitialise teture width / height / format
  void Reinitialise(GLint width, GLint height, GLint internal_format = GL_RGBA8, bool sampling_linear = true, int border = 0, GLenum glformat = GL_RGBA, GLenum gltype = GL_UNSIGNED_BYTE );

  void Bind() const;
  void Unbind() const;

  //! data_layout normally one of GL_LUMINANCE, GL_RGB, ...
  //! data_type normally one of GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_FLOAT
  void Upload(void* image, GLenum data_layout = GL_LUMINANCE, GLenum data_type = GL_FLOAT);

  void Download(void* image, GLenum data_layout = GL_LUMINANCE, GLenum data_type = GL_FLOAT) const;

  void SetLinear();
  void SetNearestNeighbour();

  void RenderToViewport(const bool flip) const;
  void RenderToViewport() const;
  void RenderToViewportFlipY() const;
  void RenderToViewportFlipXFlipY() const;

  GLint internal_format;
  GLuint tid;
  GLint width;
  GLint height;

private:
  // Private copy constructor
  GlTexture(const GlTexture&) {}
};

struct GlRenderBuffer
{
  GlRenderBuffer(GLint width, GLint height, GLint internal_format = GL_DEPTH_COMPONENT24);
  ~GlRenderBuffer();

  GLuint rbid;
};

struct GlFramebuffer
{
  GlFramebuffer();
  GlFramebuffer(GlTexture& colour, GlRenderBuffer& depth);
  GlFramebuffer(GlTexture& colour0, GlTexture& colour1, GlRenderBuffer& depth);
  ~GlFramebuffer();

  void Bind() const;
  void Unbind() const;

  // Attach Colour texture to frame buffer
  // Return attachment texture is bound to (e.g. GL_COLOR_ATTACHMENT0_EXT)
  GLenum AttachColour(GlTexture& tex);

  // Attach Depth render buffer to frame buffer
  void AttachDepth(GlRenderBuffer& rb);

  GLuint fbid;
  unsigned attachments;
};

enum GlBufferType
{
  GlArrayBuffer = GL_ARRAY_BUFFER,                    // VBO's, CBO's, NBO's
  GlElementArrayBuffer = GL_ELEMENT_ARRAY_BUFFER,     // IBO's
  GlPixelPackBuffer = GL_PIXEL_PACK_BUFFER,           // PBO's
  GlPixelUnpackBuffer = GL_PIXEL_UNPACK_BUFFER
};

struct GlBuffer
{
  //! Default constructor represents 'no buffer'
  GlBuffer();
  GlBuffer(GlBufferType buffer_type, GLuint num_elements, GLenum datatype, GLuint count_per_element, GLenum gluse = GL_DYNAMIC_DRAW );
  
#if __cplusplus > 199711L
  //! Move Constructor
  GlBuffer(GlBuffer&& tex);
#endif  
  
  ~GlBuffer();
  
  void Reinitialise(GlBufferType buffer_type, GLuint num_elements, GLenum datatype, GLuint count_per_element, GLenum gluse );
  void Resize(GLuint num_elements);

  void Bind() const;
  void Unbind() const;
  void Upload(const GLvoid* data, GLsizeiptr size_bytes, GLintptr offset = 0);

  GLuint bo;
  GlBufferType buffer_type;
  GLenum gluse;

  GLenum datatype;
  GLuint num_elements;
  GLuint count_per_element;
private:
  GlBuffer(const GlBuffer&) {}
};

size_t GlDataTypeBytes(GLenum type);

void glColorHSV( double hue, double s, double v );

void glColorBin( int bin, int max_bins, double sat = 1.0, double val = 1.0 );

void glPixelTransferScale( float r, float g, float b );
void glPixelTransferScale( float scale );

////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////

inline void _CheckGlDieOnError( const char *sFile, const int nLine )
{
    GLenum glError = glGetError();
    if( glError != GL_NO_ERROR ) {
        std::cerr << "OpenGL Error: " << sFile << ":" << nLine << std::endl;
        std::cerr << gluErrorString(glError) << std::endl;
        exit( -1 );
    }
}

const int MAX_ATTACHMENTS = 8;
const static GLuint attachment_buffers[] = {
    GL_COLOR_ATTACHMENT0_EXT,
    GL_COLOR_ATTACHMENT1_EXT,
    GL_COLOR_ATTACHMENT2_EXT,
    GL_COLOR_ATTACHMENT3_EXT,
    GL_COLOR_ATTACHMENT4_EXT,
    GL_COLOR_ATTACHMENT5_EXT,
    GL_COLOR_ATTACHMENT6_EXT,
    GL_COLOR_ATTACHMENT7_EXT
};

const static size_t datatype_bytes[] = {
    1, //  #define GL_BYTE 0x1400
    1, //  #define GL_UNSIGNED_BYTE 0x1401
    2, //  #define GL_SHORT 0x1402
    2, //  #define GL_UNSIGNED_SHORT 0x1403
    4, //  #define GL_INT 0x1404
    4, //  #define GL_UNSIGNED_INT 0x1405
    4, //  #define GL_FLOAT 0x1406
    2, //  #define GL_2_BYTES 0x1407
    3, //  #define GL_3_BYTES 0x1408
    4, //  #define GL_4_BYTES 0x1409
    8  //  #define GL_DOUBLE 0x140A
};

inline size_t GlDataTypeBytes(GLenum type)
{
    return datatype_bytes[type - GL_BYTE];
}


//template<typename T>
//struct GlDataTypeTrait {};
//template<> struct GlDataTypeTrait<float>{ static const GLenum type = GL_FLOAT; };
//template<> struct GlDataTypeTrait<int>{ static const GLenum type = GL_INT; };
//template<> struct GlDataTypeTrait<unsigned char>{ static const GLenum type = GL_UNSIGNED_BYTE; };

inline GlTexture::GlTexture()
    : internal_format(0), tid(0), width(0), height(0)
{
  // Not a texture constructor
}

inline GlTexture::GlTexture(GLint width, GLint height, GLint internal_format, bool sampling_linear, int border, GLenum glformat, GLenum gltype )
    : internal_format(0), tid(0)
{
  Reinitialise(width,height,internal_format,sampling_linear,border,glformat,gltype);
}

#if __cplusplus > 199711L
  inline GlTexture::GlTexture(GlTexture&& tex)
      : internal_format(tex.internal_format), tid(tex.tid)
  {
      tex.internal_format = 0;
      tex.tid = 0;
  }
#endif

inline GlTexture::~GlTexture()
{
  if(internal_format!=0) {
    glDeleteTextures(1,&tid);
  }
}

inline void GlTexture::Bind() const
{
  glBindTexture(GL_TEXTURE_2D, tid);
}

inline void GlTexture::Unbind() const
{
  glBindTexture(GL_TEXTURE_2D, 0);
}

inline void GlTexture::Reinitialise(GLint w, GLint h, GLint int_format, bool sampling_linear, int border, GLenum glformat, GLenum gltype )
{
    if(tid!=0) {
      glDeleteTextures(1,&tid);
    }

    internal_format = int_format;
    width = w;
    height = h;

    glGenTextures(1,&tid);
    Bind();
    // GL_LUMINANCE and GL_FLOAT don't seem to actually affect buffer, but some values are required
    // for call to succeed.
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, border, glformat, gltype, 0);

    if(sampling_linear) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }else{
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

inline void GlTexture::Upload(void* image, GLenum data_layout, GLenum data_type )
{
  Bind();
  glTexSubImage2D(GL_TEXTURE_2D,0,0,0,width,height,data_layout,data_type,image);
}

inline void GlTexture::Download(void* image, GLenum data_layout, GLenum data_type) const
{
  Bind();
  glGetTexImage(GL_TEXTURE_2D, 0, data_layout, data_type, image);
  Unbind();
}


inline void GlTexture::SetLinear()
{
  Bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  Unbind();
}

inline void GlTexture::SetNearestNeighbour()
{
  Bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  Unbind();
}

inline void GlTexture::RenderToViewport(const bool flip) const
{
    if(flip) {
        RenderToViewportFlipY();
    }else{
        RenderToViewport();
    }
}

inline void GlTexture::RenderToViewport() const
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  Bind();
  glEnable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
  glTexCoord2f(0, 0); glVertex2d(-1,-1);
  glTexCoord2f(1, 0); glVertex2d(1,-1);
  glTexCoord2f(1, 1); glVertex2d(1,1);
  glTexCoord2f(0, 1); glVertex2d(-1,1);
  glEnd();
  glDisable(GL_TEXTURE_2D);
}

inline void GlTexture::RenderToViewportFlipY() const
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  Bind();
  glEnable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
  glTexCoord2f(0, 0); glVertex2d(-1,1);
  glTexCoord2f(1, 0); glVertex2d(1,1);
  glTexCoord2f(1, 1); glVertex2d(1,-1);
  glTexCoord2f(0, 1); glVertex2d(-1,-1);
  glEnd();
  glDisable(GL_TEXTURE_2D);
}

inline void GlTexture::RenderToViewportFlipXFlipY() const
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  Bind();
  glEnable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
  glTexCoord2f(0, 0); glVertex2d(1,1);
  glTexCoord2f(1, 0); glVertex2d(-1,1);
  glTexCoord2f(1, 1); glVertex2d(-1,-1);
  glTexCoord2f(0, 1); glVertex2d(1,-1);
  glEnd();
  glDisable(GL_TEXTURE_2D);
}

inline GlRenderBuffer::GlRenderBuffer(GLint width, GLint height, GLint internal_format )
{
  glGenRenderbuffersEXT(1, &rbid);
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rbid);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, internal_format, width, height);
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
}

inline GlRenderBuffer::~GlRenderBuffer()
{
  glDeleteRenderbuffersEXT(1, &rbid);
}

inline GlFramebuffer::GlFramebuffer()
  : attachments(0)
{
  glGenFramebuffersEXT(1, &fbid);
}

inline GlFramebuffer::GlFramebuffer(GlTexture& colour, GlRenderBuffer& depth)
    : attachments(0)
{
  glGenFramebuffersEXT(1, &fbid);
  AttachColour(colour);
  AttachDepth(depth);
  CheckGlDieOnError();
}

inline GlFramebuffer::GlFramebuffer(GlTexture& colour0, GlTexture& colour1, GlRenderBuffer& depth)
    : attachments(0)
{
  glGenFramebuffersEXT(1, &fbid);
  AttachColour(colour0);
  AttachColour(colour1);
  AttachDepth(depth);
  CheckGlDieOnError();
}

inline GlFramebuffer::~GlFramebuffer()
{
  glDeleteFramebuffersEXT(1, &fbid);
}

inline void GlFramebuffer::Bind() const
{
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbid);
  glDrawBuffers( attachments, attachment_buffers );
}

inline void GlFramebuffer::Unbind() const
{
  glDrawBuffers( 1, attachment_buffers );
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

inline GLenum GlFramebuffer::AttachColour(GlTexture& tex )
{
    const GLenum color_attachment = GL_COLOR_ATTACHMENT0_EXT + attachments;
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbid);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, color_attachment, GL_TEXTURE_2D, tex.tid, 0);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    attachments++;
    CheckGlDieOnError();
    return color_attachment;
}

inline void GlFramebuffer::AttachDepth(GlRenderBuffer& rb )
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbid);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, rb.rbid);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    CheckGlDieOnError();
}

inline GlBuffer::GlBuffer()
    : bo(0), num_elements(0)
{
}

inline GlBuffer::GlBuffer(GlBufferType buffer_type, GLuint num_elements, GLenum datatype, GLuint count_per_element, GLenum gluse )
    : bo(0), num_elements(0)
{
    Reinitialise(buffer_type, num_elements, datatype, count_per_element, gluse );
}

#if __cplusplus > 199711L
  inline GlBuffer::GlBuffer(GlBuffer&& buffer)
      : bo(buffer.bo), buffer_type(buffer.buffer_type), gluse(buffer.gluse), datatype(buffer.datatype),
        num_elements(buffer.num_elements), count_per_element(buffer.count_per_element)
  {
      buffer.bo = 0;
  }
#endif

inline void GlBuffer::Reinitialise(GlBufferType buffer_type, GLuint num_elements, GLenum datatype, GLuint count_per_element, GLenum gluse )
{
  this->buffer_type = buffer_type;
  this->gluse = gluse;
  this->datatype = datatype;
  this->num_elements = num_elements;
  this->count_per_element = count_per_element;
    
  if(!bo) {
    glGenBuffers(1, &bo);
  }

  Bind();
  glBufferData(buffer_type, num_elements*GlDataTypeBytes(datatype)*count_per_element, 0, gluse);
  Unbind();
}

inline void GlBuffer::Resize(GLuint new_num_elements)
{
    if(bo!=0) {
        // Backup current data, reinit memory, restore old data
        const size_t backup_elements = std::min(new_num_elements,num_elements);
        const size_t backup_size_bytes = backup_elements*GlDataTypeBytes(datatype)*count_per_element;
        unsigned char* backup = new unsigned char[backup_size_bytes];
        Bind();
        glGetBufferSubData(buffer_type, 0, backup_size_bytes, backup);
        glBufferData(buffer_type, new_num_elements*GlDataTypeBytes(datatype)*count_per_element, 0, gluse);
        glBufferSubData(buffer_type, 0, backup_size_bytes, backup);
        Unbind();
        delete[] backup;
    }else{
        Reinitialise(buffer_type, new_num_elements, datatype, count_per_element, gluse);
    }
    num_elements = new_num_elements;
}

inline GlBuffer::~GlBuffer()
{
  if(bo!=0) {
    glDeleteBuffers(1, &bo);
  }
}

inline void GlBuffer::Bind() const
{
  glBindBuffer(buffer_type, bo);
}

inline void GlBuffer::Unbind() const
{
  glBindBuffer(buffer_type, 0);
}

inline void GlBuffer::Upload(const GLvoid* data, GLsizeiptr size_bytes, GLintptr offset)
{
  Bind();
  glBufferSubData(buffer_type,offset,size_bytes, data);
}

}

#endif // PANGOLIN_GL_H
