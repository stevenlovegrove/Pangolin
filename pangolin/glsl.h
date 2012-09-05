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

#ifndef PANGOLIN_GLSL_H
#define PANGOLIN_GLSL_H

#include <sstream>
#include <algorithm>

#include "gl.h"

#ifdef HAVE_EIGEN
#include <Eigen/Eigen>
#endif // HAVE_EIGEN

namespace pangolin
{

////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////

enum GlSlShaderType
{
    GlSlFragmentShader = GL_FRAGMENT_SHADER_ARB,
    GlSlVertexShader = GL_VERTEX_SHADER_ARB
};

class GlSlProgram
{
public:
    GlSlProgram();
    ~GlSlProgram();

    void AddShader(GlSlShaderType shader_type, const std::string& source_code);
    void Link();

    void SetUniform(const std::string& name, GlTexture& tex);
    void SetUniform(const std::string& name, float f);
    void SetUniform(const std::string& name, float f1, float f2, float f3, float f4);

    void Bind();
    void Unbind();

protected:
    bool linked;
    std::vector<GLhandleARB> shaders;
    GLenum prog;
};

class GlSlUtilities
{
public:
    inline static GlSlProgram& Scale(float scale, float bias = 0.0f) {
        GlSlProgram& prog = Instance().prog_scale;
        prog.Bind();
        prog.SetUniform("scale", scale);
        prog.SetUniform("bias",  bias);
    }

    inline static void UseNone()
    {
        glUseProgram(0);
    }

protected:
    static GlSlUtilities* instance;
    static GlSlUtilities& Instance() {
        if(!instance) {
            instance = new GlSlUtilities();
        }
        return *instance;
    }

    // protected constructor
    GlSlUtilities() {
        const char* source = "uniform float scale; uniform float bias; uniform sampler2D tex; void main() { gl_FragColor = texture2D(tex,gl_TexCoord[0].st); gl_FragColor.xyz *= scale; }";
        prog_scale.AddShader(GlSlFragmentShader, source);
        prog_scale.Link();
    }

    GlSlProgram prog_scale;
};


////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////

GlSlUtilities* GlSlUtilities::instance = 0;

void printInfoLog(GLhandleARB obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB,
                     &infologLength);

    if (infologLength > 0)
    {
    infoLog = (char *)malloc(infologLength);
    glGetInfoLogARB(obj, infologLength, &charsWritten, infoLog);
    printf("%s\n",infoLog);
    free(infoLog);
    }
}

inline GlSlProgram::GlSlProgram()
    : linked(false)
{
    prog = glCreateProgramObjectARB();
}

inline GlSlProgram::~GlSlProgram()
{
    // Remove and delete each shader
    for(int i=0; i<shaders.size(); ++i ) {
        glDetachObjectARB(prog, shaders[i]);
        glDeleteObjectARB(shaders[i]);
    }
    glDeleteProgramsARB(1, &prog);
}

inline void GlSlProgram::AddShader(GlSlShaderType shader_type, const std::string& source_code)
{
    GLhandleARB shader = glCreateShaderObjectARB(shader_type);
    const char* source = source_code.c_str();
    glShaderSourceARB(shader, 1, &source, NULL);
    glCompileShader(shader);
    glAttachObjectARB(prog, shader);
    printInfoLog(shader);

    shaders.push_back(shader);
    linked = false;
}

inline void GlSlProgram::Link()
{
    glLinkProgram(prog);
    printInfoLog(prog);
}

inline void GlSlProgram::Bind()
{
    glUseProgram(prog);
}

inline void GlSlProgram::Unbind()
{
    glUseProgram(0);
}

inline void GlSlProgram::SetUniform(const std::string& name, float f)
{
    GLint location = glGetUniformLocationARB(prog, name.c_str());
    glUniform1f(location,f);
}

inline void GlSlProgram::SetUniform(const std::string& name, float f1, float f2, float f3, float f4)
{
    GLint location = glGetUniformLocationARB(prog, name.c_str());
    glUniform4f(location,f1,f2,f3,f4);
}

}

#endif // PANGOLIN_CG_H
