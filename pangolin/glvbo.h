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

#ifndef PANGOLIN_GLVBO_H
#define PANGOLIN_GLVBO_H

#include <pangolin/gl.h>

namespace pangolin
{

////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////

void RenderVbo(pangolin::GlBuffer& vbo, int w, int h);

void RenderVboCbo(pangolin::GlBuffer& vbo, pangolin::GlBuffer& cbo, int w, int h, bool draw_color = true);

void RenderVboIbo(pangolin::GlBuffer& vbo, pangolin::GlBuffer& ibo, int w, int h, bool draw_mesh = true);

void RenderVboIboCbo(pangolin::GlBuffer& vbo, pangolin::GlBuffer& ibo, pangolin::GlBuffer& cbo, int w, int h, bool draw_mesh = true, bool draw_color = true);

void RenderVboIboCboNbo(pangolin::GlBuffer& vbo, pangolin::GlBuffer& ibo, pangolin::GlBuffer& cbo, pangolin::GlBuffer& nbo, int w, int h, bool draw_mesh = true, bool draw_color = true, bool draw_normals = true);

////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////

inline void RenderVbo(pangolin::GlBuffer& vbo, int w, int h)
{
    vbo.Bind();
    glVertexPointer(vbo.count_per_element, vbo.datatype, 0, 0);
    glEnableClientState(GL_VERTEX_ARRAY);

    glPointSize(2.0);
    glDrawArrays(GL_POINTS, 0, w * h);

    glDisableClientState(GL_VERTEX_ARRAY);
    vbo.Unbind();

}

inline void RenderVboCbo(pangolin::GlBuffer& vbo, pangolin::GlBuffer& cbo, int w, int h, bool draw_color)
{
    if(draw_color) {
        cbo.Bind();
        glColorPointer(cbo.count_per_element, cbo.datatype, 0, 0);
        glEnableClientState(GL_COLOR_ARRAY);
    }

    RenderVbo(vbo,w,h);

    if(draw_color) {
        glDisableClientState(GL_COLOR_ARRAY);
        cbo.Unbind();
    }
}

inline void RenderVboIbo(pangolin::GlBuffer& vbo, pangolin::GlBuffer& ibo, int w, int h, bool draw_mesh)
{
    vbo.Bind();
    glVertexPointer(vbo.count_per_element, vbo.datatype, 0, 0);
    glEnableClientState(GL_VERTEX_ARRAY);

    if(draw_mesh) {
        ibo.Bind();
        for( int r=0; r<h-1; ++r) {
            glDrawElements(GL_TRIANGLE_STRIP,w*ibo.count_per_element, ibo.datatype, (unsigned int*)0 + ibo.width*ibo.count_per_element*r);
        }
        ibo.Unbind();
    }else{
        glPointSize(2.0);
        glDrawArrays(GL_POINTS, 0, w * h);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    vbo.Unbind();
}

inline void RenderVboIboCbo(pangolin::GlBuffer& vbo, pangolin::GlBuffer& ibo, pangolin::GlBuffer& cbo, int w, int h, bool draw_mesh, bool draw_color )
{
    if(draw_color) {
        cbo.Bind();
        glColorPointer(cbo.count_per_element, cbo.datatype, 0, 0);
        glEnableClientState(GL_COLOR_ARRAY);
    }

    RenderVboIbo(vbo,ibo,w,h,draw_mesh);

    if(draw_color) {
        glDisableClientState(GL_COLOR_ARRAY);
        cbo.Unbind();
    }
}

inline void RenderVboIboCboNbo(pangolin::GlBuffer& vbo, pangolin::GlBuffer& ibo, pangolin::GlBuffer& cbo, pangolin::GlBuffer& nbo, int w, int h, bool draw_mesh, bool draw_color, bool draw_normals)
{
    if(draw_color) {
        cbo.Bind();
        glColorPointer(cbo.count_per_element, cbo.datatype, 0, 0);
        glEnableClientState(GL_COLOR_ARRAY);
    }

    vbo.Bind();
    glVertexPointer(vbo.count_per_element, vbo.datatype, 0, 0);
    glEnableClientState(GL_VERTEX_ARRAY);

    if(draw_mesh) {
        if(draw_normals) {
            nbo.Bind();
            glNormalPointer(nbo.datatype, nbo.count_per_element * pangolin::GlDataTypeBytes(nbo.datatype),0);
            glEnableClientState(GL_NORMAL_ARRAY);
        }

        ibo.Bind();
        for( int r=0; r<h-1; ++r) {
            glDrawElements(GL_TRIANGLE_STRIP,w*ibo.count_per_element, ibo.datatype, (unsigned int*)0 + ibo.width*ibo.count_per_element*r);
        }
        ibo.Unbind();

        if(draw_normals) {
            glDisableClientState(GL_NORMAL_ARRAY);
            nbo.Unbind();
        }
    }else{
        glPointSize(2.0);
        glDrawArrays(GL_POINTS, 0, w * h);
    }

    if(draw_color) {
        glDisableClientState(GL_COLOR_ARRAY);
        cbo.Unbind();
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    vbo.Unbind();
}

}

#endif // PANGOLIN_GLVBO_H
