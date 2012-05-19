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
#include <GL/gl.h>

#include <math.h>

namespace pangolin
{

////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////

struct GlTexture
{
  //! internal_format normally one of GL_RGBA8, GL_LUMINANCE8
  GlTexture(GLint width, GLint height, GLint internal_format = GL_RGBA8 );
  ~GlTexture();

  void Bind() const;
  void Unbind() const;

  //! data_layout normally one of GL_LUMINANCE, GL_RGB, ...
  //! data_type normally one of GL_UNSIGNED_BYTE, GL_FLOAT
  void Upload(void* image, GLenum data_layout = GL_LUMINANCE, GLenum data_type = GL_FLOAT);

  void SetLinear();
  void SetNearestNeighbour();

  void RenderToViewport() const;
  void RenderToViewportFlipY() const;

  GLint internal_format;
  GLuint tid;
  GLint width;
  GLint height;
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

  GLuint fbid;
  unsigned attachments;
};

void glColorHSV( double hue, double s, double v );

void glColorBin( int bin, int max_bins, double sat = 1.0, double val = 1.0 );

void glPixelTransferScale( float r, float g, float b );
void glPixelTransferScale( float scale );

////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////

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

//template<typename T>
//struct GlDataTypeTrait {};
//template<> struct GlDataTypeTrait<float>{ static const GLenum type = GL_FLOAT; };
//template<> struct GlDataTypeTrait<int>{ static const GLenum type = GL_INT; };
//template<> struct GlDataTypeTrait<unsigned char>{ static const GLenum type = GL_UNSIGNED_BYTE; };

inline GlTexture::GlTexture(GLint width, GLint height, GLint internal_format)
  : internal_format(internal_format),width(width),height(height)
{
  glGenTextures(1,&tid);
  Bind();
  // GL_LUMINANCE and GL_FLOAT don't seem to actually affect buffer, but some values are required
  // for call to succeed.
  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, GL_LUMINANCE,GL_FLOAT,0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}

inline GlTexture::~GlTexture()
{
  glDeleteTextures(1,&tid);
}

inline void GlTexture::Bind() const
{
  glBindTexture(GL_TEXTURE_2D, tid);
}

inline void GlTexture::Unbind() const
{
  glBindTexture(GL_TEXTURE_2D, 0);
}

inline void GlTexture::Upload(void* image, GLenum data_layout, GLenum data_type )
{
  Bind();
  glTexSubImage2D(GL_TEXTURE_2D,0,0,0,width,height,data_layout,data_type,image);
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



inline void GlTexture::RenderToViewport() const
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  Bind();
  glEnable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
  glTexCoord2f(0, 0);
  glVertex2d(-1,-1);
  glTexCoord2f(1, 0);
  glVertex2d(1,-1);
  glTexCoord2f(1, 1);
  glVertex2d(1,1);
  glTexCoord2f(0, 1);
  glVertex2d(-1,1);
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
  glTexCoord2f(0, 0);
  glVertex2d(-1,1);
  glTexCoord2f(1, 0);
  glVertex2d(1,1);
  glTexCoord2f(1, 1);
  glVertex2d(1,-1);
  glTexCoord2f(0, 1);
  glVertex2d(-1,-1);
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
{
  glGenFramebuffersEXT(1, &fbid);
}

inline GlFramebuffer::GlFramebuffer(GlTexture& colour, GlRenderBuffer& depth)
{
  glGenFramebuffersEXT(1, &fbid);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbid);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, colour.tid, 0);
  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depth.rbid);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  attachments = 1;
}

inline GlFramebuffer::GlFramebuffer(GlTexture& colour0, GlTexture& colour1, GlRenderBuffer& depth)
{
  glGenFramebuffersEXT(1, &fbid);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbid);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, colour0.tid, 0);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, colour1.tid, 0);
  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depth.rbid);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  attachments = 2;
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

// h [0,360)
// s [0,1]
// v [0,1]
inline void glColorHSV( double hue, double s, double v )
{
  const double h = hue / 60.0;
  const int i = floor(h);
  const double f = (i%2 == 0) ? 1-(h-i) : h-i;
  const double m = v * (1-s);
  const double n = v * (1-s*f);
  switch(i)
  {
  case 0: glColor3d(v,n,m); break;
  case 1: glColor3d(n,v,m); break;
  case 2: glColor3d(m,v,n); break;
  case 3: glColor3d(m,n,v); break;
  case 4: glColor3d(n,m,v); break;
  case 5: glColor3d(v,m,n); break;
  default:
    break;
  }

}

inline void glColorBin( int bin, int max_bins, double sat, double val )
{
  if( bin >= 0 )
  {
    const double hue = (double)(bin%max_bins) * 360.0 / (double)max_bins;
    glColorHSV(hue,sat,val);
  }else{
    glColor3f(1,1,1);
  }
}

inline void glPixelTransferScale( float r, float g, float b )
{
    glPixelTransferf(GL_RED_SCALE,r);
    glPixelTransferf(GL_GREEN_SCALE,g);
    glPixelTransferf(GL_BLUE_SCALE,b);
}

inline void glPixelTransferScale( float scale )
{
    glPixelTransferScale(scale,scale,scale);
}



}

#endif // PANGOLIN_GL_H
