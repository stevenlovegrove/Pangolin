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

#ifdef HAVE_EIGEN
#include <Eigen/Eigen>
#endif // HAVE_EIGEN

namespace pangolin
{

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

inline void glDraw_z0(float scale, int grid)
{
    const float maxord = grid*scale;
    glBegin(GL_LINES);
    for(int i=-grid; i<=grid; ++i )
    {
        glVertex2f(i*scale,-maxord);
        glVertex2f(i*scale,+maxord);
        glVertex2f(-maxord, i*scale);
        glVertex2f(+maxord, i*scale);
    }
    glEnd();
}

inline void glDrawLine( float x1, float y1, float x2, float y2 )
{
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

inline void glDrawCross( float x, float y, int r = 5 )
{
    glBegin(GL_LINES);
    glVertex2f(x,y-r);
    glVertex2f(x,y+r);
    glVertex2f(x-r,y);
    glVertex2f(x+r,y);
    glEnd();
}

inline void glDrawCross( float x, float y, float z, int r )
{
    glBegin(GL_LINES);
    glVertex3f(x,y-r,z);
    glVertex3f(x,y+r,z);
    glVertex3f(x-r,y,z);
    glVertex3f(x+r,y,z);
    glVertex3f(x,y,z-r);
    glVertex3f(x,y,z+r);
    glEnd();
}

inline void glDrawCircle( float x, float y, double radius )
{
    glBegin(GL_POLYGON);
    for( double a=0; a< 2*M_PI; a += M_PI/50.0 )
    {
        glVertex2d( x + radius * cos(a), y + radius * sin(a) );
    }
    glEnd();
}

inline void glDrawAxis(float s)
{
    glBegin(GL_LINES);
    glColor3f(1,0,0);
    glVertex3f(0,0,0);
    glVertex3f(s,0,0);
    glColor3f(0,1,0);
    glVertex3f(0,0,0);
    glVertex3f(0,s,0);
    glColor3f(0,0,1);
    glVertex3f(0,0,0);
    glVertex3f(0,0,s);
    glEnd();
}

#ifdef HAVE_EIGEN
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
#endif // HAVE_EIGEN

}

#endif // PANGOLIN_GLDRAW_H
