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

#include <pangolin/glplatform.h>

#ifdef HAVE_GLES
    #define GLhandleARB GLuint
#endif

#if defined(HAVE_EIGEN) && !defined(__CUDACC__) //prevent including Eigen in cuda files
#define USE_EIGEN
#endif

#ifdef USE_EIGEN
#include <Eigen/Eigen>
#endif // USE_EIGEN

namespace pangolin
{

////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////

enum GlSlShaderType
{
    GlSlFragmentShader = GL_FRAGMENT_SHADER,
    GlSlVertexShader = GL_VERTEX_SHADER
};

class GlSlProgram
{
public:
    GlSlProgram();
    ~GlSlProgram();
    
    void AddShader(GlSlShaderType shader_type, const std::string& source_code);
    void Link();
    
    GLint GetAttributeHandle(const std::string& name);
    GLint GetUniformHandle(const std::string& name);

//    void SetUniform(const std::string& name, GlTexture& tex);
    void SetUniform(const std::string& name, float f);
    void SetUniform(const std::string& name, float f1, float f2);
    void SetUniform(const std::string& name, float f1, float f2, float f3);
    void SetUniform(const std::string& name, float f1, float f2, float f3, float f4);

    
    void Bind();
    void SaveBind();
    void Unbind();
    
protected:
    bool linked;
    std::vector<GLhandleARB> shaders;
    GLenum prog;

    GLint prev_prog;
};

class GlSlUtilities
{
public:
    inline static GlSlProgram& Scale(float scale, float bias = 0.0f) {
        GlSlProgram& prog = Instance().prog_scale;
        prog.Bind();
        prog.SetUniform("scale", scale);
        prog.SetUniform("bias",  bias);
        return prog;
    }
    
    inline static void UseNone()
    {
        glUseProgram(0);
    }
    
protected:
    static GlSlUtilities& Instance() {
        static GlSlUtilities instance;
        return instance;
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

inline void printProgInfoLog(GLhandleARB prog)
{
    GLint status;
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if(status != GL_TRUE) {
        print_error("GL_LINK_STATUS != GL_TRUE");
        const int PROGRAM_LOG_MAX_LEN = 1024;
        char infolog[PROGRAM_LOG_MAX_LEN];
        GLsizei len;
        glGetProgramInfoLog(prog, PROGRAM_LOG_MAX_LEN, &len, infolog);
        if(len) print_error("%s\n",infolog);
    }
}

inline void printShaderInfoLog(GLhandleARB shader)
{
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(status != GL_TRUE) {
        print_error("GL_COMPILE_STATUS != GL_TRUE");
        const int SHADER_LOG_MAX_LEN = 1024;
        char infolog[SHADER_LOG_MAX_LEN];
        GLsizei len;
        glGetShaderInfoLog(shader, SHADER_LOG_MAX_LEN, &len, infolog);
        if(len) print_error("%s\n",infolog);
    }
}

inline GlSlProgram::GlSlProgram()
    : linked(false), prev_prog(0)
{
    prog = glCreateProgram();
}

inline GlSlProgram::~GlSlProgram()
{
    // Remove and delete each shader
    for(size_t i=0; i<shaders.size(); ++i ) {
        glDetachShader(prog, shaders[i]);
        glDeleteShader(shaders[i]);
    }
    glDeleteProgram(prog);
}

inline void GlSlProgram::AddShader(GlSlShaderType shader_type, const std::string& source_code)
{
    GLhandleARB shader = glCreateShader(shader_type);
    const char* source = source_code.c_str();
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    printShaderInfoLog(shader);
    glAttachShader(prog, shader);

    shaders.push_back(shader);
    linked = false;
}

inline void GlSlProgram::Link()
{
    glLinkProgram(prog);
    printProgInfoLog(prog);
}

inline void GlSlProgram::Bind()
{
    prev_prog = 0;
    glUseProgram(prog);
}

inline void GlSlProgram::SaveBind()
{
    glGetIntegerv(GL_CURRENT_PROGRAM, &prev_prog);
    glUseProgram(prog);
}

inline void GlSlProgram::Unbind()
{
    glUseProgram(prev_prog);
}

inline GLint GlSlProgram::GetAttributeHandle(const std::string& name)
{
    return glGetAttribLocation(prog, name.c_str());
}

inline GLint GlSlProgram::GetUniformHandle(const std::string& name)
{
    return glGetUniformLocation(prog, name.c_str());
}

inline void GlSlProgram::SetUniform(const std::string& name, float f)
{
    glUniform1f( GetUniformHandle(name), f);
}

inline void GlSlProgram::SetUniform(const std::string& name, float f1, float f2)
{
    glUniform2f( GetUniformHandle(name), f1,f2);
}

inline void GlSlProgram::SetUniform(const std::string& name, float f1, float f2, float f3)
{
    glUniform3f( GetUniformHandle(name), f1,f2,f3);
}

inline void GlSlProgram::SetUniform(const std::string& name, float f1, float f2, float f3, float f4)
{
    glUniform4f( GetUniformHandle(name), f1,f2,f3,f4);
}

}

#endif // PANGOLIN_CG_H
