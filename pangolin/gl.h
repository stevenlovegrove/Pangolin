#ifndef PANGOLIN_GL_H
#define PANGOLIN_GL_H

#include "platform.h"
#include <GL/gl.h>

namespace pangolin
{

////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////

enum GlDataLayout
{
  GlDataLayoutLuminance = GL_LUMINANCE,
  GlDataLayoutRGB = GL_RGB,
  GlDataLayoutRGBA = GL_RGBA,
  GlDataLayoutLuminanceAlpha = GL_LUMINANCE_ALPHA
};

struct GlTexture
{
  GlTexture(GLint width, GLint height, GLenum channels);
  ~GlTexture();

  void Bind() const;

  template<typename T>
  void Upload(T* image, GlDataLayout data_layout = GlDataLayoutLuminance );

  void RenderToViewport() const;

  GLuint tid;
  GLint width;
  GLint height;
  GLenum channels;
};


////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////

template<typename T>
struct GlDataType {};

template<> struct GlDataType<float>{ static const GLenum type = GL_FLOAT; };
template<> struct GlDataType<int>{ static const GLenum type = GL_INT; };
template<> struct GlDataType<unsigned char>{ static const GLenum type = GL_UNSIGNED_BYTE; };

inline GlTexture::GlTexture(GLint width, GLint height, GLenum channels)
  :width(width),height(height),channels(channels)
{
  glGenTextures(1,&tid);
  Bind();
  glTexImage2D(GL_TEXTURE_2D, 0, channels, width, height, 0, 0,0,0);
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

template<typename T>
inline void GlTexture::Upload(T* image, GlDataLayout data_layout )
{
  Bind();
  glTexImage2D(GL_TEXTURE_2D, 0, channels, width, height, 0, data_layout, GlDataType<T>::type, image);
//  glTexSubImage2D(GL_TEXTURE_2D,0,0,0,width,height,data_layout,GlDataType<T>::type,image);
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


}

#endif // PANGOLIN_GL_H
