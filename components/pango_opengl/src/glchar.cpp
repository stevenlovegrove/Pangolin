/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2013 Steven Lovegrove
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

#include <pangolin/gl/glchar.h>

namespace pangolin
{

GlChar::GlChar()
    : x_step(0.0f)
{
    // Uninitialised
}

// x and y are in 'discrete' coords, where (0,0) is center of top-left pixel.
GlChar::GlChar(size_t atlas_index, int tw, int th, GLfloat x, GLfloat y, GLfloat w, GLfloat h, GLfloat advance, GLfloat ox, GLfloat oy)
    : x_step(advance), atlas_index(atlas_index)
{
    // Convert to u,v tex coords \in [0,1]
    // u,v point to the top-left corner of top-left pixel
    // (hence why we don't have the +0.5 converstion)
    const GLfloat u = x / (GLfloat)tw;
    const GLfloat v = y / (GLfloat)th;
    const GLfloat u2 = u + w / (GLfloat)tw;
    const GLfloat v2 = v + h / (GLfloat)th;

    // 0, 3
    // 1, 2

    // Setup u,v tex coords
    vs[0] = XYUV(ox,   oy,   u,  v );
    vs[1] = XYUV(ox,   oy-h, u,  v2 );
    vs[2] = XYUV(w+ox, oy-h, u2, v2 );
    vs[3] = XYUV(w+ox, oy,   u2, v );

    y_min = oy-h;
    y_max = oy;
}

void GlChar::Draw() const
{
    glVertexPointer(2, GL_FLOAT, sizeof(XYUV), &vs[0].x);
    glEnableClientState(GL_VERTEX_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(XYUV), &vs[0].tu);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_TEXTURE_2D);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

}
