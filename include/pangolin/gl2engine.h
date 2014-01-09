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

#ifndef PANGOLIN_GL2ENGINE_H
#define PANGOLIN_GL2ENGINE_H

#include <stack>

#include <pangolin/opengl_render_state.h>
#include <pangolin/glsl.h>

namespace pangolin {

class GlEngine
{
public:
    const char* vert =
            "attribute vec4 a_position;\n"
            "attribute vec4 a_color;\n"
            "attribute vec3 a_normal;\n"
            "attribute vec2 a_texcoord;\n"
            "uniform vec4 u_color;\n"
            "uniform mat4 u_modelViewMatrix;\n"
            "uniform mat4 u_modelViewProjectionMatrix;\n"
            "varying vec4 v_frontColor;\n"
            "varying vec2 v_texcoord;\n"
            "void main() {\n"
            "    gl_Position = u_modelViewProjectionMatrix * a_position;\n"
            "    v_frontColor = u_color;\n"
            "    v_texcoord = a_texcoord;\n"
            "}\n";

    const char* frag =
            "precision mediump float;\n"
            "varying vec4 v_frontColor;\n"
            "varying vec2 v_texcoord;\n"
            "uniform sampler2D u_texture;\n"
            "uniform bool u_textureEnable;\n"
            "void main() {\n"
            "  gl_FragColor = v_frontColor;\n"
            "  if(u_textureEnable) {\n"
            "    gl_FragColor *= texture2D(u_texture, v_texcoord);\n"
            "  }\n"
            "}\n";

    GlEngine()
    {
        // Initialise default state
        projection.push(IdentityMatrix());
        modelview.push(IdentityMatrix());
        currentmatrix = &modelview;

        // Set GL_TEXTURE0 as default active texture
        glActiveTexture(GL_TEXTURE0);

        // Compile and link shaders
        prog_fixed.AddShader(GlSlVertexShader, vert);
        prog_fixed.AddShader(GlSlFragmentShader, frag);
        prog_fixed.Link();

        // Save locations of attributes
        a_position = prog_fixed.GetAttributeHandle("a_position");
        a_color = prog_fixed.GetAttributeHandle("a_color");
        a_normal = prog_fixed.GetAttributeHandle("a_normal");
        a_texcoord = prog_fixed.GetAttributeHandle("a_texcoord");

        // Save locations of uniforms
        u_color = prog_fixed.GetUniformHandle("u_color");
        u_modelViewMatrix = prog_fixed.GetUniformHandle("u_modelViewMatrix");
        u_modelViewProjectionMatrix = prog_fixed.GetUniformHandle("u_modelViewProjectionMatrix");
        u_texture = prog_fixed.GetUniformHandle("u_texture");
        u_textureEnable = prog_fixed.GetUniformHandle("u_textureEnable");

        // Initialise default uniform values
        UpdateMatrices();
        SetColor(1.0,1.0,1.0,1.0);
    }

    void UpdateMatrices()
    {
        OpenGlMatrix pmv = projection.top() * modelview.top();
        GLint curprog;
        glGetIntegerv(GL_CURRENT_PROGRAM, &curprog);
        prog_fixed.Bind();
        glUniformMatrix4fv( u_modelViewMatrix, 1, false, modelview.top().m );
        glUniformMatrix4fv( u_modelViewProjectionMatrix, 1, false, pmv.m );
        glUseProgram(curprog);
    }

    void SetColor(float r, float g, float b, float a)
    {
        GLint curprog;
        glGetIntegerv(GL_CURRENT_PROGRAM, &curprog);
        prog_fixed.Bind();
        glUniform4f( u_color, r, g, b, a);
        glUseProgram(curprog);
    }

    void EnableTexturing(GLboolean v)
    {
        GLint curprog;
        glGetIntegerv(GL_CURRENT_PROGRAM, &curprog);
        prog_fixed.Bind();
        glUniform1i( u_textureEnable, v);
        glUseProgram(curprog);
    }

//protected:
    std::stack<OpenGlMatrix> projection;
    std::stack<OpenGlMatrix> modelview;
    std::stack<OpenGlMatrix>* currentmatrix;

    GLenum matrixmode;

    float color[4];

    GlSlProgram  prog_fixed;

    GLint a_position;
    GLint a_color;
    GLint a_normal;
    GLint a_texcoord;

    GLint u_color;
    GLint u_modelViewMatrix;
    GLint u_modelViewProjectionMatrix;
    GLint u_texture;
    GLint u_textureEnable;
};

GlEngine& glEngine();

}

///////////////////////////////////////////////////////////////////////////////
// OpenGL 1.0 compatibility - Emulate fixed pipeline
///////////////////////////////////////////////////////////////////////////////

// Missing defines that we'll be using
#define GL_MODELVIEW                      0x1700
#define GL_PROJECTION                     0x1701
#define GL_SHADE_MODEL                    0x0B54
#define GL_POINT_SIZE                     0x0B11

#define GL_MULTISAMPLE                    0x809D

#define GL_LIGHTING                       0x0B50
#define GL_POINT_SMOOTH                   0x0B10
#define GL_LINE_SMOOTH                    0x0B20
#define GL_SCISSOR_TEST                   0x0C11
#define GL_COLOR_MATERIAL                 0x0B57

#define GL_FLAT                           0x1D00
#define GL_SMOOTH                         0x1D01

#define GL_MODULATE                       0x2100
#define GL_DECAL                          0x2101
#define GL_ADD                            0x0104
#define GL_TEXTURE_ENV_MODE               0x2200
#define GL_TEXTURE_ENV_COLOR              0x2201
#define GL_TEXTURE_ENV                    0x2300

#define GL_PERSPECTIVE_CORRECTION_HINT    0x0C50
#define GL_POINT_SMOOTH_HINT              0x0C51
#define GL_LINE_SMOOTH_HINT               0x0C52

#define GL_VERTEX_ARRAY                   0x8074
#define GL_NORMAL_ARRAY                   0x8075
#define GL_COLOR_ARRAY                    0x8076
#define GL_TEXTURE_COORD_ARRAY            0x8078

inline void glEnableClientState(GLenum cap)
{
    pangolin::GlEngine& gl = pangolin::glEngine();
    if(cap == GL_VERTEX_ARRAY) {
        glEnableVertexAttribArray(gl.a_position);
    }else if(cap == GL_COLOR_ARRAY) {
        glEnableVertexAttribArray(gl.a_color);
    }else if(cap == GL_NORMAL_ARRAY) {
        glEnableVertexAttribArray(gl.a_normal);
    }else if(cap == GL_TEXTURE_COORD_ARRAY) {
        glEnableVertexAttribArray(gl.a_texcoord);
        gl.EnableTexturing(true);
    }else{
        print_error("Not Implemented: %s, %s, %d", __FUNCTION__, __FILE__, __LINE__);
    }
}

inline void glDisableClientState(GLenum cap)
{
    pangolin::GlEngine& gl = pangolin::glEngine();
    if(cap == GL_VERTEX_ARRAY) {
        glDisableVertexAttribArray(gl.a_position);
    }else if(cap == GL_COLOR_ARRAY) {
        glDisableVertexAttribArray(gl.a_color);
    }else if(cap == GL_NORMAL_ARRAY) {
        glDisableVertexAttribArray(gl.a_normal);
    }else if(cap == GL_TEXTURE_COORD_ARRAY) {
        glDisableVertexAttribArray(gl.a_texcoord);
        gl.EnableTexturing(false);
    }else{
        print_error("Not Implemented: %s, %s, %d", __FUNCTION__, __FILE__, __LINE__);
    }
}

inline void glVertexPointer( GLint size, GLenum type, GLsizei stride, const GLvoid * pointer)
{
    glVertexAttribPointer(pangolin::glEngine().a_position, size, type, GL_FALSE, stride, pointer);
}

inline void glTexCoordPointer( GLint size, GLenum type, GLsizei stride, const GLvoid * pointer)
{
    glVertexAttribPointer(pangolin::glEngine().a_texcoord, size, type, GL_FALSE, stride, pointer);
}

inline void glMatrixMode(GLenum mode)
{
    pangolin::GlEngine& gl = pangolin::glEngine();
    gl.currentmatrix = (mode == pangolin::GlProjectionStack) ? &gl.projection : &gl.modelview;
}

inline void glLoadIdentity()
{
    pangolin::GlEngine& gl = pangolin::glEngine();
    gl.currentmatrix->top() = pangolin::IdentityMatrix();
    gl.UpdateMatrices();
}

inline void glLoadMatrixf(const GLfloat* m)
{
    pangolin::GlEngine& gl = pangolin::glEngine();
    pangolin::GLprecision* cm = gl.currentmatrix->top().m;
    for(int i=0; i<16; ++i) cm[i] = (pangolin::GLprecision)m[i];
    gl.UpdateMatrices();
}

inline void glLoadMatrixd(const GLdouble* m)
{
    pangolin::GlEngine& gl = pangolin::glEngine();
    pangolin::GLprecision* cm = gl.currentmatrix->top().m;
    for(int i=0; i<16; ++i) cm[i] = (pangolin::GLprecision)m[i];
    gl.UpdateMatrices();
}

inline void glMultMatrixf(const GLfloat* m)
{
//    pangolin::GlEngine& gl = pangolin::glEngine();
//    float res[16];
//    pangolin::MatMul<4,4,4,float>(res, m, gl.currentmatrix->m );
//    std::memcpy(gl.currentmatrix->m, res, sizeof(float) * 16 );
    print_error("Not Implemented: %s, %s, %d", __FUNCTION__, __FILE__, __LINE__);
}

inline void glMultMatrixd(const GLdouble* m)
{
    print_error("Not Implemented: %s, %s, %d", __FUNCTION__, __FILE__, __LINE__);
}

inline void glPushMatrix(void)
{
    pangolin::GlEngine& gl = pangolin::glEngine();
    gl.currentmatrix->push(gl.currentmatrix->top());
}

inline void glPopMatrix(void)
{
    pangolin::GlEngine& gl = pangolin::glEngine();
    gl.currentmatrix->pop();
    gl.UpdateMatrices();
}

inline void glTranslatef(GLfloat x, GLfloat y, GLfloat z )
{
    pangolin::GlEngine& gl = pangolin::glEngine();
    pangolin::GLprecision* cm = gl.currentmatrix->top().m;
    cm[12] += x;
    cm[13] += y;
    cm[14] += z;
    gl.UpdateMatrices();
}

inline void glOrtho(
    GLdouble l, GLdouble r,
    GLdouble b, GLdouble t,
    GLdouble n, GLdouble f)
{
    pangolin::GlEngine& gl = pangolin::glEngine();
    gl.currentmatrix->top() = pangolin::ProjectionMatrixOrthographic(l,r,b,t,n,f);
    gl.UpdateMatrices();
}

inline void glColor4f(	GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    pangolin::glEngine().SetColor(red,green,blue,alpha);
}

inline void glShadeModel( GLenum mode)
{
    print_error("Not Implemented: %s, %s, %d", __FUNCTION__, __FILE__, __LINE__);
}

inline void glPointSize(GLfloat size)
{
    print_error("Not Implemented: %s, %s, %d", __FUNCTION__, __FILE__, __LINE__);
}

inline void glTexEnvf(	GLenum target,
    GLenum pname,
    GLfloat param)
{
    print_error("Not Implemented: %s, %s, %d", __FUNCTION__, __FILE__, __LINE__);
}

const GLubyte gNotErrorLookup[] = "XX";
inline const GLubyte* gluErrorString(GLenum error)
{
    return gNotErrorLookup;
}

static void __gluMultMatrixVecf(const GLfloat matrix[16], const GLfloat in[4],
                                GLfloat out[4])
{
    int i;

    for (i=0; i<4; i++)
    {
        out[i] = in[0] * matrix[0*4+i] +
                 in[1] * matrix[1*4+i] +
                 in[2] * matrix[2*4+i] +
                 in[3] * matrix[3*4+i];
    }
}

/*
** Invert 4x4 matrix.
** Contributed by David Moore (See Mesa bug #6748)
*/
static int __gluInvertMatrixf(const GLfloat m[16], GLfloat invOut[16])
{
    GLfloat inv[16], det;
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

static void __gluMultMatricesf(const GLfloat a[16], const GLfloat b[16],
                               GLfloat r[16])
{
    int i, j;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            r[i*4+j] = a[i*4+0]*b[0*4+j] +
                       a[i*4+1]*b[1*4+j] +
                       a[i*4+2]*b[2*4+j] +
                       a[i*4+3]*b[3*4+j];
        }
    }
}

inline GLint gluProject(GLfloat objx, GLfloat objy, GLfloat objz,
                        const GLfloat modelMatrix[16],
                        const GLfloat projMatrix[16],
                        const GLint viewport[4],
                        GLfloat* winx, GLfloat* winy, GLfloat* winz)
{
    GLfloat in[4];
    GLfloat out[4];

    in[0]=objx;
    in[1]=objy;
    in[2]=objz;
    in[3]=1.0;
    __gluMultMatrixVecf(modelMatrix, in, out);
    __gluMultMatrixVecf(projMatrix, out, in);
    if (in[3] == 0.0)
    {
        return(GL_FALSE);
    }

    in[0]/=in[3];
    in[1]/=in[3];
    in[2]/=in[3];
    /* Map x, y and z to range 0-1 */
    in[0]=in[0]*0.5f+0.5f;
    in[1]=in[1]*0.5f+0.5f;
    in[2]=in[2]*0.5f+0.5f;

    /* Map x,y to viewport */
    in[0]=in[0] * viewport[2] + viewport[0];
    in[1]=in[1] * viewport[3] + viewport[1];

    *winx=in[0];
    *winy=in[1];
    *winz=in[2];

    return(GL_TRUE);
}

inline GLint gluUnProject(GLfloat winx, GLfloat winy, GLfloat winz,
                          const GLfloat modelMatrix[16],
                          const GLfloat projMatrix[16],
                          const GLint viewport[4],
                          GLfloat* objx, GLfloat* objy, GLfloat* objz)
{
    GLfloat finalMatrix[16];
    GLfloat in[4];
    GLfloat out[4];

    __gluMultMatricesf(modelMatrix, projMatrix, finalMatrix);
    if (!__gluInvertMatrixf(finalMatrix, finalMatrix))
    {
        return(GL_FALSE);
    }

    in[0]=winx;
    in[1]=winy;
    in[2]=winz;
    in[3]=1.0;

    /* Map x and y from window coordinates */
    in[0] = (in[0] - viewport[0]) / viewport[2];
    in[1] = (in[1] - viewport[1]) / viewport[3];

    /* Map to range -1 to 1 */
    in[0] = in[0] * 2 - 1;
    in[1] = in[1] * 2 - 1;
    in[2] = in[2] * 2 - 1;

    __gluMultMatrixVecf(finalMatrix, in, out);
    if (out[3] == 0.0)
    {
        return(GL_FALSE);
    }

    out[0] /= out[3];
    out[1] /= out[3];
    out[2] /= out[3];
    *objx = out[0];
    *objy = out[1];
    *objz = out[2];

    return(GL_TRUE);
}



#endif // PANGOLIN_GL2ENGINE_H
