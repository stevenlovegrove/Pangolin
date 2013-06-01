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

#ifndef PANGOLIN_GLDRAW_H
#define PANGOLIN_GLDRAW_H

#include <pangolin/glinclude.h>

#ifdef HAVE_EIGEN
#include <Eigen/Eigen>
#endif // HAVE_EIGEN

namespace pangolin
{

// h [0,360)
// s [0,1]
// v [0,1]
inline void glColorHSV( GLfloat hue, GLfloat s=1.0f, GLfloat v=1.0f )
{
    const GLfloat h = hue / 60.0;
    const int i = floor(h);
    const GLfloat f = (i%2 == 0) ? 1-(h-i) : h-i;
    const GLfloat m = v * (1-s);
    const GLfloat n = v * (1-s*f);
    switch(i)
    {
    case 0: glColor4f(v,n,m,1); break;
    case 1: glColor4f(n,v,m,1); break;
    case 2: glColor4f(m,v,n,1); break;
    case 3: glColor4f(m,n,v,1); break;
    case 4: glColor4f(n,m,v,1); break;
    case 5: glColor4f(v,m,n,1); break;
    default:
        break;
    }
}

inline void glColorBin( int bin, int max_bins, GLfloat sat=1.0f, GLfloat val=1.0f )
{
    if( bin >= 0 ) {
        const GLfloat hue = (GLfloat)(bin%max_bins) * 360.0 / (GLfloat)max_bins;
        glColorHSV(hue,sat,val);
    }else{
        glColor4f(1,1,1,1);
    }
}

inline void glDrawLine( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 )
{
    GLfloat verts[] = { x1,y1,  x2,y2 };
    glVertexPointer(2, GL_FLOAT, 0, verts);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINES, 0, 2);
    glDisableClientState(GL_VERTEX_ARRAY);
}

inline void glDrawLine( GLfloat x1, GLfloat y1, GLfloat z1, GLfloat x2, GLfloat y2, GLfloat z2)
{
    GLfloat verts[] = { x1,y1,z1,  x2,y2,z2 };
    glVertexPointer(3, GL_FLOAT, 0, verts);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINES, 0, 2);
    glDisableClientState(GL_VERTEX_ARRAY);
}

inline void glDraw_z0(GLfloat scale, int grid)
{
    const GLfloat maxord = grid*scale;
    for(int i=-grid; i<=grid; ++i ) {
        glDrawLine(i*scale,-maxord,   i*scale,+maxord);
        glDrawLine(-maxord, i*scale,  +maxord, i*scale);
    }
}

inline void glDrawCross( GLfloat x, GLfloat y, GLfloat r = 5 )
{
    glDrawLine(x-r,y, x+r, y);
    glDrawLine(x,y-r, x, y+r);
}

inline void glDrawCross( GLfloat x, GLfloat y, GLfloat z, GLfloat r )
{
    glDrawLine(x-r,y,z, x+r, y,z);
    glDrawLine(x,y-r,z, x, y+r,z);
    glDrawLine(x,y,z-r, x, y,z+r);
}

inline void glDrawCircle( GLfloat x, GLfloat y, GLfloat rad )
{
    GLfloat verts[720];
    
    for(int i = 0; i < 720; i+=2) {
        verts[i] = x + rad * cos(M_PI*i/360.0);
        verts[i] = y + rad * sin(M_PI*i/360.0);
    }
    
    glVertexPointer(2, GL_FLOAT, 0, verts);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 360);
    glDisableClientState(GL_VERTEX_ARRAY);
}

inline void glDrawCirclePerimeter( float x, float y, double rad )
{
    GLfloat verts[720];
    
    for(int i = 0; i < 720; i+=2) {
        verts[i] = x + rad * cos(M_PI*i/360.0);
        verts[i] = y + rad * sin(M_PI*i/360.0);
    }
    
    glVertexPointer(2, GL_FLOAT, 0, verts);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINES, 0, 360);
    glDisableClientState(GL_VERTEX_ARRAY);
}

inline void glDrawAxis(float s)
{
    glColor4f(1,0,0,1);
    glDrawLine(0,0,0, s,0,0);

    glColor4f(0,1,0,1);
    glDrawLine(0,0,0, 0,s,0);
    
    glColor4f(0,0,1,1);
    glDrawLine(0,0,0, 0,0,s);
}

#ifdef HAVE_EIGEN

#ifndef HAVE_GLES
inline void glVertex( const Eigen::Vector3d& p )
{
    glVertex3dv(p.data());
}
#endif

inline void glDrawLine( const Eigen::Vector2d& p1, const Eigen::Vector2d& p2 )
{
    glDrawLine(p1(0), p1(1), p2(0), p2(1));
}

inline void glDrawCross( const Eigen::Vector2d& p, int r = 5 )
{
    glDrawCross(p(0), p(1), r);
}

inline void glDrawCross( const Eigen::Vector3d& p, int r = 5 )
{
    glDrawCross(p(0), p(1), p(2), r);
}

inline void glDrawCircle( const Eigen::Vector2d& p, double radius = 5 )
{
    glDrawCircle(p(0), p(1), radius );
}

inline void glDrawCirclePerimeter( const Eigen::Vector2d& p, double radius = 5 )
{
    glDrawCirclePerimeter(p(0), p(1), radius);
}

inline void glSetFrameOfReference( const Eigen::Matrix4d& T_wf )
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
#ifndef HAVE_GLES
    glMultMatrixd( T_wf.data() );
#else
    const Eigen::Matrix4f fT_wf = T_wf.cast<GLfloat>();
    glMultMatrixf( fT_wf.data() );
#endif
}

inline void glUnsetFrameOfReference()
{
    glPopMatrix();
}

inline void glDrawAxis( const Eigen::Matrix4d& T_wf, float scale )
{
    glSetFrameOfReference(T_wf);
    glDrawAxis(scale);
    glUnsetFrameOfReference();
}

inline void glDrawFrustrum( const Eigen::Matrix3d& Kinv, int w, int h, GLfloat scale )
{
    const GLfloat fu = scale*Kinv(0,0);
    const GLfloat fv = scale*Kinv(1,1);
    const GLfloat u0 = scale*Kinv(0,2);
    const GLfloat v0 = scale*Kinv(0,2);
    
    GLfloat verts[] = {
        u0, v0, scale,
        fu+u0, v0, scale,
        w*fu+u0, h*fv+v0, scale,
        u0, h*fv+v0, scale,
        u0, v0, scale,
        0, 0, 0,
        w*fu+u0, h*fv+v0, scale,
        fu+u0, v0, scale,
        u0, h*fv+v0, scale
    };
    glVertexPointer(2, GL_FLOAT, 0, verts);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINES, 0, 9);
    glDisableClientState(GL_VERTEX_ARRAY);
}

inline void glDrawFrustrum( const Eigen::Matrix3d& Kinv, int w, int h, const Eigen::Matrix4d& T_wf, float scale )
{
    glSetFrameOfReference(T_wf);
    glDrawFrustrum(Kinv,w,h,scale);
    glUnsetFrameOfReference();
}

#endif // HAVE_EIGEN

#ifndef HAVE_GLES
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
#endif

}

#endif // PANGOLIN_GLDRAW_H
