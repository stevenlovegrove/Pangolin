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

#include <pangolin/opengl_render_state.h>
#include <pangolin/glsl.h>

namespace pangolin {

class GlEngine
{
public:
    const char* vert =
            "attribute vec4 a_position;\n"
            "attribute vec4 a_color;\n"
//            "attribute vec3 a_normal;\n"
//            "attribute vec2 a_texcoord;\n"
//            "uniform bool u_normalEnabled;\n"
//            "uniform bool u_colorEnabled;\n"
            "uniform vec4 u_color;\n"
            "uniform mat4 u_modelViewMatrix;\n"
            "uniform mat4 u_modelViewProjectionMatrix;\n"
//            "uniform mat3 u_transposeAdjointModelViewMatrix;\n"
            "varying vec4 v_frontColor;\n"
//            "varying vec3 v_normal;\n"
            "void main() {\n"
            "    gl_Position = u_modelViewProjectionMatrix * a_position;\n"
//            "    gl_Position = a_position;\n"
//            "    v_frontColor = u_colorEnabled ? a_color : u_color;\n"
            "    v_frontColor = u_color;\n"
//            "    v_normal = u_transposeAdjointModelViewMatrix * a_normal;\n"
            "}\n";

    const char* frag =
            "precision mediump float;\n"
            "varying vec4 v_frontColor;\n"
//            "varying vec3 v_normal;\n"
            "void main() {\n"
            "  gl_FragColor = v_frontColor;\n"
//            "  gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
            "}\n";

    GlEngine()
    {
        // Initialise default state
        projection = IdentityMatrix();
        modelview = IdentityMatrix();
        currentmatrix = &modelview;

        // Compile and link shaders
        prog_fixed.AddShader(GlSlVertexShader, vert);
        prog_fixed.AddShader(GlSlFragmentShader, frag);
        prog_fixed.Link();

        // Save locations of attributes
        a_position = prog_fixed.GetAttributeHandle("a_position");
//        a_color = prog_fixed.GetAttributeHandle("a_color");
//        a_normal = prog_fixed.GetAttributeHandle("a_normal");
//        a_texcoord = prog_fixed.GetAttributeHandle("a_texcoord");

        // Save locations of uniforms
        // TODO

        // Initialise default uniform values
        prog_fixed.Bind();
//        glUniform1f( prog_fixed.GetUniformHandle("u_colorEnabled"), 0.0);
        glUniform4f( prog_fixed.GetUniformHandle("u_color"), 1.0, 0.0, 0.0, 1.0);
        prog_fixed.Unbind();

        UpdateMatrices();
    }

    void UpdateMatrices()
    {
        OpenGlMatrix pmv = projection * modelview;
        GLint curprog;
        glGetIntegerv(GL_CURRENT_PROGRAM, &curprog);
        prog_fixed.Bind();
        glUniformMatrix4fv( prog_fixed.GetUniformHandle("u_modelViewMatrix"), 1, false, modelview.m );
        glUniformMatrix4fv( prog_fixed.GetUniformHandle("u_modelViewProjectionMatrix"), 1, false, pmv.m );
        glUseProgram(curprog);
    }

    void UpdateColors()
    {
        GLint curprog;
        glGetIntegerv(GL_CURRENT_PROGRAM, &curprog);
        prog_fixed.Bind();
        glUniform4f( prog_fixed.GetUniformHandle("u_color"), color[0], color[1], color[2], color[3]);
        glUseProgram(curprog);
    }

//protected:
    OpenGlMatrix projection;
    OpenGlMatrix modelview;
    OpenGlMatrix* currentmatrix;

    GLenum matrixmode;

    float color[4];

    GlSlProgram  prog_fixed;
    GLint a_position;
    GLint a_color;
    GLint a_normal;
    GLint a_texcoord;
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
#define GL_MODELVIEW_MATRIX               0x0BA6
#define GL_PROJECTION_MATRIX              0x0BA7
#define GL_TEXTURE_MATRIX                 0x0BA8

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
    if(cap == GL_VERTEX_ARRAY) {
        glEnableVertexAttribArray(0);
    }else{
        print_error("Not Implemented: %s, %d", __FILE__, __LINE__);
    }
}

inline void glDisableClientState(GLenum cap)
{
    if(cap == GL_VERTEX_ARRAY) {
        glDisableVertexAttribArray(0);
    }else{
        print_error("Not Implemented: %s, %d", __FILE__, __LINE__);
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
    *gl.currentmatrix = pangolin::IdentityMatrix();
    gl.UpdateMatrices();
}

inline void glLoadMatrixf(const GLfloat* m)
{
    pangolin::GlEngine& gl = pangolin::glEngine();
    pangolin::GLprecision* cm = gl.currentmatrix->m;
    for(int i=0; i<16; ++i) cm[i] = (pangolin::GLprecision)m[i];
    gl.UpdateMatrices();
}

inline void glLoadMatrixd(const GLdouble* m)
{
    pangolin::GlEngine& gl = pangolin::glEngine();
    pangolin::GLprecision* cm = gl.currentmatrix->m;
    for(int i=0; i<16; ++i) cm[i] = (pangolin::GLprecision)m[i];
    gl.UpdateMatrices();
}

inline void glMultMatrixf(const GLfloat* m)
{
//    pangolin::GlEngine& gl = pangolin::glEngine();
//    float res[16];
//    pangolin::MatMul<4,4,4,float>(res, m, gl.currentmatrix->m );
//    std::memcpy(gl.currentmatrix->m, res, sizeof(float) * 16 );
    print_error("Not Implemented: %s, %d", __FILE__, __LINE__);
}

inline void glMultMatrixd(const GLdouble* m)
{
    print_error("Not Implemented: %s, %d", __FILE__, __LINE__);
}

inline void glColor4f(	GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    pangolin::GlEngine& gl = pangolin::glEngine();
    gl.color[0] = red;
    gl.color[1] = green;
    gl.color[2] = blue;
    gl.color[3] = alpha;
    gl.UpdateColors();
}

inline void glShadeModel( GLenum mode)
{
    print_error("Not Implemented: %s, %s, %d", __FUNCTION__, __FILE__, __LINE__);
}

inline void glPointSize(GLfloat size)
{
    print_error("Not Implemented: %s, %s, %d", __FUNCTION__, __FILE__, __LINE__);
}

inline void glPushMatrix(void)
{
    print_error("Not Implemented: %s, %s, %d", __FUNCTION__, __FILE__, __LINE__);
}

inline void glPopMatrix(void)
{
    print_error("Not Implemented: %s, %s, %d", __FUNCTION__, __FILE__, __LINE__);
}

inline void glTranslatef(GLfloat x, GLfloat y, GLfloat z )
{
    print_error("Not Implemented: %s, %s, %d", __FUNCTION__, __FILE__, __LINE__);
}

inline void glTexEnvf(	GLenum target,
    GLenum pname,
    GLfloat param)
{
    print_error("Not Implemented: %s, %s, %d", __FUNCTION__, __FILE__, __LINE__);
}

inline const GLubyte* gluErrorString(GLenum error)
{
    print_error("Not Implemented: %s, %s, %d", __FUNCTION__, __FILE__, __LINE__);
    return 0;
}

inline void gluOrtho2D(int x1, int x2, int y1, int y2)
{
    print_error("Not Implemented: %s, %s, %d", __FUNCTION__, __FILE__, __LINE__);
}

inline GLint gluUnProject(
    GLdouble winX,
    GLdouble winY,
    GLdouble winZ,
    const GLdouble* model,
    const GLdouble* proj,
    const GLint* view,
    GLdouble* objX,
    GLdouble* objY,
    GLdouble* objZ)
{
    print_error("Not Implemented: %s, %s, %d", __FUNCTION__, __FILE__, __LINE__);
    return 0;
}

inline GLint gluProject(	GLdouble  	objX,
    GLdouble  	objY,
    GLdouble  	objZ,
    const GLdouble *  	model,
    const GLdouble *  	proj,
    const GLint *  	view,
    GLdouble*  	winX,
    GLdouble*  	winY,
    GLdouble*  	winZ)
{
    print_error("Not Implemented: %s, %s, %d", __FUNCTION__, __FILE__, __LINE__);
    return 0;
}


#endif // PANGOLIN_GL2ENGINE_H
