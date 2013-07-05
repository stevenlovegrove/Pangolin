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
#define glGetDoublev                glGetFloatv

inline void glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
#ifndef HAVE_GLES
    glRectf(x1,y1, x2,y2);
#else
    GLfloat verts[] = { x1,y1,  x2,y1,  x2,y2,  x1,y2 };    
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, verts);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
#endif
}

inline void glRecti(int x1, int y1, int x2, int y2)
{
#ifndef HAVE_GLES
    glRecti(x1,y1, x2,y2);
#else
    GLfloat verts[] = { (float)x1,(float)y1,  (float)x2,(float)y1,
                        (float)x2,(float)y2,  (float)x1,(float)y2 };    
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, verts);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
#endif
}

// We dont want a program to fail just because of a test teapot. Draw placeholder.
#define glutWireTeapot(X) (pangolin::glDrawColouredCube(-(X),X))

#endif // PANGOLIN_GL_ES_COMPAT_H
