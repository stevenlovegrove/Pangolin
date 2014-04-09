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
#include <fstream>
#include <algorithm>
#include <vector>
#include <map>

#include <pangolin/glplatform.h>
#include <pangolin/colour.h>
#include <pangolin/file_utils.h>

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
// Standard attribute locations
////////////////////////////////////////////////

const GLuint DEFAULT_LOCATION_POSITION = 0;
const GLuint DEFAULT_LOCATION_COLOUR   = 1;
const GLuint DEFAULT_LOCATION_NORMAL   = 2;
const GLuint DEFAULT_LOCATION_TEXCOORD = 3;

const char DEFAULT_NAME_POSITION[] = "a_position";
const char DEFAULT_NAME_COLOUR[]   = "a_color";
const char DEFAULT_NAME_NORMAL[]   = "a_normal";
const char DEFAULT_NAME_TEXCOORD[] = "a_texcoord";

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

#ifdef CALLEE_HAS_RVALREF
    //! Move Constructor
    GlSlProgram(GlSlProgram&& tex);
#endif

    ~GlSlProgram();
    
    void AddShader(
        GlSlShaderType shader_type,
        const std::string& source_code,
        const std::map<std::string,std::string>& program_defines = std::map<std::string,std::string>(),
        const std::vector<std::string>& search_path = std::vector<std::string>()
    );

    void AddShaderFromFile(
        GlSlShaderType shader_type,
        const std::string& filename,
        const std::map<std::string,std::string>& program_defines = std::map<std::string,std::string>(),
        const std::vector<std::string>& search_path = std::vector<std::string>()
    );

    void Link();
    
    GLint GetAttributeHandle(const std::string& name);
    GLint GetUniformHandle(const std::string& name);

    // Before setting uniforms, be sure to Bind() the GlSl program first.
    void SetUniform(const std::string& name, int x);
    void SetUniform(const std::string& name, int x1, int x2);
    void SetUniform(const std::string& name, int x1, int x2, int x3);
    void SetUniform(const std::string& name, int x1, int x2, int x3, int x4);

    void SetUniform(const std::string& name, float f);
    void SetUniform(const std::string& name, float f1, float f2);
    void SetUniform(const std::string& name, float f1, float f2, float f3);
    void SetUniform(const std::string& name, float f1, float f2, float f3, float f4);

    void SetUniform(const std::string& name, Colour c);
    
    void Bind();
    void SaveBind();
    void Unbind();

    void BindPangolinDefaultAttribLocationsAndLink();

protected:
    std::string ParseIncludeFilename(
        const std::string& location
    );

    std::string SearchIncludePath(
        const std::string& filename,
        const std::vector<std::string>& search_path,
        const std::string& current_path
    );

    void AddPreprocessedShader(
        GlSlShaderType shader_type,
        const std::string& source_code
    );

    void ParseGLSL(
        std::istream& input,
        std::ostream& output,
        const std::map<std::string,std::string>& program_defines,
        const std::vector<std::string>& search_path,
        const std::string& current_path
    );

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
        const char* source = "uniform float scale; uniform float bias; uniform sampler2D tex; void main() { gl_FragColor = texture2D(tex,gl_TexCoord[0].st); gl_FragColor.xyz *= scale; gl_FragColor.xyz += vec3(bias,bias,bias); }";
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
        pango_print_error("GL_LINK_STATUS != GL_TRUE");
        const int PROGRAM_LOG_MAX_LEN = 1024;
        char infolog[PROGRAM_LOG_MAX_LEN];
        GLsizei len;
        glGetProgramInfoLog(prog, PROGRAM_LOG_MAX_LEN, &len, infolog);
        if(len) pango_print_error("%s\n",infolog);
    }
}

inline void printShaderInfoLog(GLhandleARB shader)
{
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(status != GL_TRUE) {
        pango_print_error("GL_COMPILE_STATUS: ");
        const int SHADER_LOG_MAX_LEN = 1024;
        char infolog[SHADER_LOG_MAX_LEN];
        GLsizei len;
        glGetShaderInfoLog(shader, SHADER_LOG_MAX_LEN, &len, infolog);
        if(len) pango_print_error("%s\n",infolog);
    }
}

inline GlSlProgram::GlSlProgram()
    : linked(false), prog(0), prev_prog(0)
{
}

#ifdef CALLEE_HAS_RVALREF
//! Move Constructor
inline GlSlProgram::GlSlProgram(GlSlProgram&& o)
    : linked(o.linked), shaders(o.shaders), prog(o.prog), prev_prog(o.prev_prog)
{
    o.prog = 0;
}
#endif


inline GlSlProgram::~GlSlProgram()
{
    if(prog) {
        // Remove and delete each shader
        for(size_t i=0; i<shaders.size(); ++i ) {
            glDetachShader(prog, shaders[i]);
            glDeleteShader(shaders[i]);
        }
        glDeleteProgram(prog);
    }
}

inline void GlSlProgram::AddPreprocessedShader(
    GlSlShaderType shader_type,
    const std::string& source_code
) {
    if(!prog) {
        prog = glCreateProgram();
    }

    GLhandleARB shader = glCreateShader(shader_type);
    const char* source = source_code.c_str();
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    printShaderInfoLog(shader);
    glAttachShader(prog, shader);

    shaders.push_back(shader);
    linked = false;
}

inline std::string GlSlProgram::ParseIncludeFilename(const std::string& location)
{
    size_t start = location.find_first_of("\"<");
    if(start != std::string::npos) {
        size_t end = location.find_first_of("\">", start+1);
        if(end != std::string::npos) {
            return location.substr(start+1, end - start - 1);
        }
    }
    throw std::runtime_error("GLSL Parser: Unable to parse include location " + location );
}

inline std::string GlSlProgram::SearchIncludePath(
    const std::string& filename,
    const std::vector<std::string>& search_path,
    const std::string& current_path
) {
    if(FileExists(current_path + "/" + filename)) {
        return current_path + "/" + filename;
    }else{
        for(size_t i=0; i < search_path.size(); ++i) {
            const std::string hypoth = search_path[i] + "/" + filename;
            if( FileExists(hypoth) ) {
                return hypoth;
            }
        }
    }
    return "";
}

inline void GlSlProgram::ParseGLSL(
        std::istream& input, std::ostream& output,
        const std::map<std::string,std::string>& program_defines,
        const std::vector<std::string> &search_path,
        const std::string &current_path
) {
    const size_t MAXLINESIZE = 10240;
    char line[MAXLINESIZE];

    while(!input.eof()) {
        // Take like from source
        input.getline(line,MAXLINESIZE);

        // Transform
        if( !strncmp(line, "#include", 8 ) ) {
            // C++ / G3D style include directive
            const std::string import_file = ParseIncludeFilename(line+8);
            const std::string resolved_file = SearchIncludePath(import_file, search_path, current_path);

            std::ifstream ifs(resolved_file.c_str());
            if(ifs.good()) {
                const std::string file_path = pangolin::PathParent(resolved_file);
                ParseGLSL(ifs, output, program_defines, search_path, file_path);
            }else{
                throw std::runtime_error("GLSL Parser: Unable to open " + import_file );
            }
        }else if( !strncmp(line, "#expect", 7) ) {
            // G3D style 'expect' directive, annotating expected preprocessor
            // definition with document string
            size_t token_start = 7;
            while( std::isspace(line[token_start]) ) ++token_start;
            size_t token_end = token_start;
            while( !std::isspace(line[token_end]) ) ++token_end;
            std::string token(line+token_start, line+token_end);
            std::map<std::string,std::string>::const_iterator it = program_defines.find(token);
            if( it == program_defines.end() ) {
                pango_print_warn("Expected define missing (defaulting to 0): %s\n%s\n", token.c_str(), line + token_end );
                output << "#define " << token << " 0" << std::endl;
            }else{
                output << "#define " << token << " " << it->second << std::endl;
            }
        }else{
            // Output directly
            output << line << std::endl;
        }
    }
}

inline void GlSlProgram::AddShader(
    GlSlShaderType shader_type,
    const std::string& source_code,
    const std::map<std::string,std::string>& program_defines,
    const std::vector<std::string>& search_path
) {
    std::istringstream iss(source_code);
    std::stringstream buffer;
    ParseGLSL(iss, buffer, program_defines, search_path, ".");
    AddPreprocessedShader(shader_type, buffer.str() );
}

inline void GlSlProgram::AddShaderFromFile(
    GlSlShaderType shader_type,
    const std::string& filename,
    const std::map<std::string,std::string>& program_defines,
    const std::vector<std::string>& search_path
) {
    std::ifstream ifs(filename.c_str());
    std::stringstream buffer;
    ParseGLSL(ifs, buffer, program_defines, search_path, ".");
    AddPreprocessedShader(shader_type, buffer.str() );
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

inline void GlSlProgram::SetUniform(const std::string& name, int x)
{
    glUniform1i( GetUniformHandle(name), x);
}

inline void GlSlProgram::SetUniform(const std::string& name, int x1, int x2)
{
    glUniform2i( GetUniformHandle(name), x1, x2);
}

inline void GlSlProgram::SetUniform(const std::string& name, int x1, int x2, int x3)
{
    glUniform3i( GetUniformHandle(name), x1, x2, x3);
}

inline void GlSlProgram::SetUniform(const std::string& name, int x1, int x2, int x3, int x4)
{
    glUniform4i( GetUniformHandle(name), x1, x2, x3, x4);
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

inline void GlSlProgram::SetUniform(const std::string& name, Colour c)
{
    glUniform4f( GetUniformHandle(name), c.r, c.g, c.b, c.a);
}

inline void GlSlProgram::BindPangolinDefaultAttribLocationsAndLink()
{
    glBindAttribLocation(prog, DEFAULT_LOCATION_POSITION, DEFAULT_NAME_POSITION);
    glBindAttribLocation(prog, DEFAULT_LOCATION_COLOUR,   DEFAULT_NAME_COLOUR);
    glBindAttribLocation(prog, DEFAULT_LOCATION_NORMAL,   DEFAULT_NAME_NORMAL);
    glBindAttribLocation(prog, DEFAULT_LOCATION_TEXCOORD, DEFAULT_NAME_TEXCOORD);
    Link();
}

}

#endif // PANGOLIN_CG_H
