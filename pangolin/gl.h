#ifndef PANGOLIN_GL_H
#define PANGOLIN_GL_H

#include "platform.h"
#include <GL/gl.h>

namespace pangolin
{

////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////

struct GlTexture
{
  GlTexture(GLint width, GLint height, GLint internal_format = GL_RGBA32F );
  ~GlTexture();

  void Bind() const;
  void Unbind() const;

  void Upload(void* image, GLenum data_layout = GL_LUMINANCE, GLenum data_type = GL_FLOAT);

  void RenderToViewport() const;
  void RenderToViewportFlipY() const;

  GLint internal_format;
  GLuint tid;
  GLint width;
  GLint height;
};


////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////

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


}

#endif // PANGOLIN_GL_H
