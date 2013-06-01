#ifndef PANGOLIN_GL_ES_COMPAT_H
#define PANGOLIN_GL_ES_COMPAT_H

#define GLdouble     GLfloat
#define glClearDepth glClearDepthf
#define glOrtho      glOrthof
#define glFrustum    glFrustumf

#define glColor4fv(a)       glColor4f(a[0], a[1], a[2], a[3])
#define glColor3fv(a)       glColor4f(a[0], a[1], a[2], 1.0f)
#define glColor3f(a,b,c)    glColor4f(a, b, c, 1.0f)

#define GL_CLAMP                    GL_CLAMP_TO_EDGE
#define GL_DEPTH_COMPONENT24        GL_DEPTH_COMPONENT24_OES
#define GL_FRAMEBUFFER_EXT          GL_FRAMEBUFFER_OES
#define GL_COLOR_ATTACHMENT0_EXT    GL_COLOR_ATTACHMENT0_OES

#define glGenFramebuffersEXT        glGenFramebuffersOES
#define glDeleteFramebuffersEXT     glDeleteFramebuffersOES
#define glBindFramebufferEXT        glBindFramebufferOES
#define glDrawBuffers               glDrawBuffersOES
#define glFramebufferTexture2DEXT   glFramebufferTexture2DOES

#endif // PANGOLIN_GL_ES_COMPAT_H
