/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2014 Steven Lovegrove
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

#include <pangolin/glpangoglu.h>
#include <pangolin/simple_math.h>

namespace pangolin {

const GLubyte gNotErrorLookup[] = "XX";

const GLubyte* glErrorString(GLenum /*error*/)
{
    // TODO: Implement glErrorString
    return gNotErrorLookup;
}

// Based on glu implementation.
template<typename P>
int InvertMatrix(const P m[16], P invOut[16])
{
    P inv[16], det;
    int i;

    inv[0] =   m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15]
             + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
    inv[4] =  -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15]
             - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
    inv[8] =   m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15]
             + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
    inv[12] = -m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14]
             - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
    inv[1] =  -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15]
             - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
    inv[5] =   m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15]
             + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
    inv[9] =  -m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15]
             - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
    inv[13] =  m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14]
             + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
    inv[2] =   m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15]
             + m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
    inv[6] =  -m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15]
             - m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
    inv[10] =  m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15]
             + m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
    inv[14] = -m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14]
             - m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];
    inv[3] =  -m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11]
             - m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];
    inv[7] =   m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11]
             + m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];
    inv[11] = -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11]
             - m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];
    inv[15] =  m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10]
             + m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];

    det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
    if (det == 0)
        return GL_FALSE;

    det=1.0f/det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return GL_TRUE;
}

// Based on glu implementation
GLint glProject(
    GLfloat objx, GLfloat objy, GLfloat objz,
    const GLfloat modelMatrix[16],
    const GLfloat projMatrix[16],
    const GLint viewport[4],
    GLfloat* winx, GLfloat* winy, GLfloat* winz)
{
    GLfloat t1[4] = {objx, objy, objz, 1.0f};
    GLfloat t2[4];

    MatMul<4,4,1,GLfloat>(t2, modelMatrix, t1);
    MatMul<4,4,1,GLfloat>(t1, projMatrix, t2);

    if (t1[3] == 0.0) {
        return(GL_FALSE);
    }

    // Normalise
    t1[0]/=t1[3];
    t1[1]/=t1[3];
    t1[2]/=t1[3];

    // Map x, y and z to range 0-1
    t1[0]=t1[0]*0.5f+0.5f;
    t1[1]=t1[1]*0.5f+0.5f;
    t1[2]=t1[2]*0.5f+0.5f;

    // Map x,y to viewport
    t1[0]=t1[0] * viewport[2] + viewport[0];
    t1[1]=t1[1] * viewport[3] + viewport[1];

    *winx=t1[0];
    *winy=t1[1];
    *winz=t1[2];

    return GL_TRUE;
}

// Based on glu implementation
GLint glUnProject(
    GLfloat winx, GLfloat winy, GLfloat winz,
    const GLfloat mv[16],
    const GLfloat proj[16],
    const GLint viewport[4],
    GLfloat* objx, GLfloat* objy, GLfloat* objz)
{
    GLfloat t1[16];

    MatMul<4,4,4,GLfloat>(t1, proj, mv);

    if (!InvertMatrix<GLfloat>(t1, t1)) {
        return(GL_FALSE);
    }

    // Map x and y from window coordinates
    GLfloat in[4] = {winx, winy, winz, 1.0f};
    in[0] = (in[0] - viewport[0]) / viewport[2];
    in[1] = (in[1] - viewport[1]) / viewport[3];

    // Map to range -1 to 1
    in[0] = in[0] * 2 - 1;
    in[1] = in[1] * 2 - 1;
    in[2] = in[2] * 2 - 1;

    GLfloat out[4];
    MatMul<4,4,1,GLfloat>(out, t1, in);

    if (out[3] == 0.0) {
        return(GL_FALSE);
    }

    // Normalise
    out[0] /= out[3];
    out[1] /= out[3];
    out[2] /= out[3];

    // Copy out
    *objx = out[0];
    *objy = out[1];
    *objz = out[2];

    return GL_TRUE;
}

// Based on glu implementation
GLint glProject(
    GLdouble objx, GLdouble objy, GLdouble objz,
    const GLdouble modelMatrix[16],
    const GLdouble projMatrix[16],
    const GLint viewport[4],
    GLdouble* winx, GLdouble* winy, GLdouble* winz)
{
    GLdouble t1[4] = {objx, objy, objz, 1.0f};
    GLdouble t2[4];

    MatMul<4,4,1,GLdouble>(t2, modelMatrix, t1);
    MatMul<4,4,1,GLdouble>(t1, projMatrix, t2);

    if (t1[3] == 0.0) {
        return(GL_FALSE);
    }

    // Normalise
    t1[0]/=t1[3];
    t1[1]/=t1[3];
    t1[2]/=t1[3];

    // Map x, y and z to range 0-1
    t1[0]=t1[0]*0.5f+0.5f;
    t1[1]=t1[1]*0.5f+0.5f;
    t1[2]=t1[2]*0.5f+0.5f;

    // Map x,y to viewport
    t1[0]=t1[0] * viewport[2] + viewport[0];
    t1[1]=t1[1] * viewport[3] + viewport[1];

    *winx=t1[0];
    *winy=t1[1];
    *winz=t1[2];

    return GL_TRUE;
}

// Based on glu implementation
GLint glUnProject(
    GLdouble winx, GLdouble winy, GLdouble winz,
    const GLdouble mv[16],
    const GLdouble proj[16],
    const GLint viewport[4],
    GLdouble* objx, GLdouble* objy, GLdouble* objz)
{
    GLdouble t1[16];

    MatMul<4,4,4,GLdouble>(t1, proj, mv);

    if (!InvertMatrix<GLdouble>(t1, t1)) {
        return(GL_FALSE);
    }

    // Map x and y from window coordinates
    GLdouble in[4] = {winx, winy, winz, 1.0f};
    in[0] = (in[0] - viewport[0]) / viewport[2];
    in[1] = (in[1] - viewport[1]) / viewport[3];

    // Map to range -1 to 1
    in[0] = in[0] * 2 - 1;
    in[1] = in[1] * 2 - 1;
    in[2] = in[2] * 2 - 1;

    GLdouble out[4];
    MatMul<4,4,1,GLdouble>(out, t1, in);

    if (out[3] == 0.0) {
        return(GL_FALSE);
    }

    // Normalise
    out[0] /= out[3];
    out[1] /= out[3];
    out[2] /= out[3];

    // Copy out
    *objx = out[0];
    *objy = out[1];
    *objz = out[2];

    return GL_TRUE;
}


}
