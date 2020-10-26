#pragma once

#include <pangolin/platform.h>

#define GLdouble     GLfloat
#define glClearDepth glClearDepthf
#define glFrustum    glFrustumf

#define glColor4fv(a)       glColor4f(a[0], a[1], a[2], a[3])
#define glColor3fv(a)       glColor4f(a[0], a[1], a[2], 1.0f)
#define glColor3f(a,b,c)    glColor4f(a, b, c, 1.0f)

#define glGenFramebuffersEXT        glGenFramebuffers
#define glDeleteFramebuffersEXT     glDeleteFramebuffers
#define glBindFramebufferEXT        glBindFramebuffer
#define glFramebufferTexture2DEXT   glFramebufferTexture2D

#define glGetDoublev                glGetFloatv

#include <pangolin/gl/compat/gl2engine.h>

inline void glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    GLfloat verts[] = { x1,y1,  x2,y1,  x2,y2,  x1,y2 };    
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, verts);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
}

inline void glRecti(int x1, int y1, int x2, int y2)
{
    GLfloat verts[] = { (float)x1,(float)y1,  (float)x2,(float)y1,
                        (float)x2,(float)y2,  (float)x1,(float)y2 };    
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, verts);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
}
