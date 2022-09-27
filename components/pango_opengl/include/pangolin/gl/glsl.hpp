#pragma once

#include <pangolin/gl/glsl.h>
#include <unordered_map>

////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////

namespace pangolin
{

// forward declaration of map build by cmake (into shaders.cpp)
const std::unordered_map<std::string,const char*>& GetBuiltinShaders();

inline bool IsLinkSuccessPrintLog(GLhandleARB prog)
{
    GLint status;
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if(status != GL_TRUE) {
        pango_print_error("GLSL Program link failed: ");
        const int PROGRAM_LOG_MAX_LEN = 10240;
        char infolog[PROGRAM_LOG_MAX_LEN];
        GLsizei len;
        glGetProgramInfoLog(prog, PROGRAM_LOG_MAX_LEN, &len, infolog);
        if(len) {
            pango_print_error("%s\n",infolog);
        }else{
            pango_print_error("No details provided.\n");
        }
        return false;
    }
    return true;
}

inline std::string AddLineNumbers(const std::string& code)
{
    std::istringstream f(code);
    std::ostringstream out;
    std::string line;
    size_t line_num = 0;
    while (std::getline(f, line)) {
        out << ++line_num << ": " << line << std::endl;
    }
    return out.str();
}

inline bool IsCompileSuccessPrintLog(GLhandleARB shader, const std::string& name_for_errors, const std::string& source_code = {})
{
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(status != GL_TRUE) {
        pango_print_error("GLSL Shader compilation failed: ");
        const int SHADER_LOG_MAX_LEN = 10240;
        char infolog[SHADER_LOG_MAX_LEN];
        GLsizei len;
        glGetShaderInfoLog(shader, SHADER_LOG_MAX_LEN, &len, infolog);
        if(len) {
            pango_print_error("%s:\n%s\n",name_for_errors.c_str(), infolog);
        }else{
            pango_print_error("%s:\nNo details provided.\n",name_for_errors.c_str());
        }
        if(!source_code.empty())
        {
            pango_print_error("In source code:\n%s\n", AddLineNumbers(source_code).c_str());
        }
        return false;
    }
    return true;
}

inline GlSlProgram::GlSlProgram()
    : linked(false), prog(0), prev_prog(0)
{
}

//! Move Constructor
inline GlSlProgram::GlSlProgram(GlSlProgram&& o)
    : linked(o.linked), shaders(o.shaders), prog(o.prog), prev_prog(o.prev_prog)
{
    o.prog = 0;
}

inline GlSlProgram::~GlSlProgram()
{
    if(prog) {
        ClearShaders();
        glDeleteProgram(prog);
    }
}

inline void PrintSourceCode(const std::string& src)
{
    std::stringstream ss(src);
    std::string line;

    for(int linenum=1; std::getline(ss,line,'\n'); ++linenum) {
        std::cout << linenum << ":\t" << line << std::endl;
    }
}

inline bool GlSlProgram::AddPreprocessedShader(
    GlSlShaderType shader_type,
    const std::string& source_code,
    const std::string& name_for_errors
    ) {
    if(!prog) {
        prog = glCreateProgram();
    }

    GLhandleARB shader = glCreateShader(shader_type);
    const char* source = source_code.c_str();
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    bool success = IsCompileSuccessPrintLog(shader, name_for_errors, source_code);
    if(success) {
        glAttachShader(prog, shader);
        shaders.push_back(shader);
        linked = false;
    }
    return success;
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

// Version of pangolin::FileExists that also looks in the builtin_shaders map
inline bool ShaderFileExists(const std::string& filename)
{
    auto it = GetBuiltinShaders().find(filename);
    return it != GetBuiltinShaders().end() || FileExists(filename);
}

inline std::shared_ptr<std::istream> GlSlProgram::OpenShaderFile(const std::string& filename)
{
    auto it = GetBuiltinShaders().find(filename);
    if(it != GetBuiltinShaders().end()) {
        return std::shared_ptr<std::istream>( new std::istringstream(it->second) );
    }else{
        return std::shared_ptr<std::istream>( new std::ifstream(filename.c_str()) );
    }
}

inline std::string GlSlProgram::SearchIncludePath(
    const std::string& filename,
    const std::vector<std::string>& search_path,
    const std::string& current_path
    ) {
    if(ShaderFileExists(current_path + "/" + filename)) {
        return current_path + "/" + filename;
    }else{
        for(size_t i=0; i < search_path.size(); ++i) {
            const std::string hypoth = search_path[i] + "/" + filename;
            if( ShaderFileExists(hypoth) ) {
                return hypoth;
            }
        }
    }
    return "";
}


inline void GlSlProgram::PreprocessGLSL(
    std::istream& input, std::ostream& output,
    const std::map<std::string,std::string>& program_defines,
    const std::vector<std::string> &search_path,
    const std::string &current_path
    ) {
    const size_t MAXLINESIZE = 10240;
    char line[MAXLINESIZE] = "";

    while(!input.eof()) {
        // Take like from source
        input.getline(line,MAXLINESIZE);

        // Transform
        if( !strncmp(line, "#include", 8 ) ) {
            // C++ / G3D style include directive
            const std::string import_file = ParseIncludeFilename(line+8);
            const std::string resolved_file = SearchIncludePath(import_file, search_path, current_path);

            auto ifs = OpenShaderFile(resolved_file);
            if(ifs->good()) {
                const std::string file_path = pangolin::PathParent(resolved_file);
                PreprocessGLSL(*ifs, output, program_defines, search_path, file_path);
            }else{
                throw std::runtime_error("GLSL Parser: Unable to open " + import_file );
            }
        }else if( !strncmp(line, "#expect", 7) ) {
            // G3D style 'expect' directive, annotating expected preprocessor
            // definition with document string

            // Consume whitespace before token
            size_t token_start = 7;
            while( std::isspace(line[token_start]) ) ++token_start;

            // Iterate over contigous charecters until \0 or whitespace
            size_t token_end = token_start;
            while( line[token_end] && !std::isspace(line[token_end]) ) ++token_end;

            std::string token(line+token_start, line+token_end);
            std::map<std::string,std::string>::const_iterator it = program_defines.find(token);
            if( it == program_defines.end() ) {
                pango_print_warn("Expected define missing (defaulting to 0): '%s'\n%s\n", token.c_str(), line + token_end );
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

inline void GlSlProgram::ClearShaders()
{
    // Remove and delete each shader
    for(size_t i=0; i<shaders.size(); ++i ) {
        glDetachShader(prog, shaders[i]);
        glDeleteShader(shaders[i]);
    }
    shaders.clear();
    linked = false;
}

inline bool GlSlProgram::AddShaderFile(const ShaderFileOrCode& shader_file)
{
    std::stringstream buffer;

    if(shader_file.code.empty()) {
        auto ifs = OpenShaderFile(shader_file.filename);
        if(ifs->good()) {
            const std::string file_path = pangolin::PathParent(shader_file.filename);
            PreprocessGLSL(*ifs, buffer, shader_file.program_defines, shader_file.search_path, file_path);
        }else{
            throw std::runtime_error(FormatString("Unable to open shader file '%'", shader_file.filename));
        }
    }else{
        std::istringstream iss(shader_file.code);
        PreprocessGLSL(iss, buffer, shader_file.program_defines, shader_file.search_path, ".");
    }

    const std::string code = buffer.str();
    const std::string input_name = !shader_file.filename.empty() ? shader_file.filename : "<string>";

    if(shader_file.shader_type == GlSlAnnotatedShader) {
        const std::map<GlSlShaderType,std::string> split_progs = SplitAnnotatedShaders(code);
        for(const auto& type_code : split_progs) {
            if(!AddPreprocessedShader(type_code.first, type_code.second, input_name )) {
                return false;
            }
        }
        return true;
    }else{
        return AddPreprocessedShader(shader_file.shader_type, code, input_name);
    }
}

inline bool GlSlProgram::AddShaderFromFile(
    GlSlShaderType shader_type,
    const std::string& filename,
    const std::map<std::string,std::string>& program_defines,
    const std::vector<std::string>& search_path
    ) {
    ShaderFileOrCode shader_file = {
        shader_type,
        pangolin::PathExpand(filename),
        std::string(),
        program_defines,
        search_path
    };
    shader_files.push_back(shader_file);
    return AddShaderFile(shader_file);
}

inline bool GlSlProgram::AddShader(
    GlSlShaderType shader_type,
    const std::string& source_code,
    const std::map<std::string,std::string>& program_defines,
    const std::vector<std::string>& search_path
    ) {
    ShaderFileOrCode shader_file = {
        shader_type,
        std::string(),
        source_code,
        program_defines,
        search_path
    };
    shader_files.push_back(shader_file);
    return AddShaderFile(shader_file);
}

inline bool GlSlProgram::ReloadShaderFiles()
{
    ClearShaders();

    for(const auto& sf : shader_files) {
        if(!AddShaderFile(sf)) {
            return false;
        }
    }

    Link();
    return true;
}

inline std::map<GlSlShaderType,std::string>
GlSlProgram::SplitAnnotatedShaders(const std::string& code)
{
    std::map<GlSlShaderType,std::string> ret;

    std::stringstream input(code);
    std::stringstream output;

    const size_t MAXLINESIZE = 10240;
    char line[MAXLINESIZE];

    GlSlShaderType current_type = GlSlAnnotatedShader;
    auto finish_block = [&](GlSlShaderType type){
        if(current_type != GlSlAnnotatedShader) {
            ret[current_type] = output.str();
        }
        output.str(std::string());
        current_type = type;
    };

    while(!input.eof()) {
        // Take like from source
        input.getline(line,MAXLINESIZE);

        // Transform
        if( !strncmp(line, "@start", 6 ) ) {
            const std::string str_shader_type = pangolin::Trim(std::string(line).substr(6));
            if(str_shader_type == "vertex") {
                finish_block(GlSlVertexShader);
            }else if(str_shader_type == "fragment") {
                finish_block(GlSlFragmentShader);
            }else if(str_shader_type == "geometry") {
                finish_block(GlSlGeometryShader);
            }else if(str_shader_type == "compute") {
                finish_block(GlSlComputeShader);
            }
        }else{
            output << line << std::endl;
        }
    }

    finish_block(GlSlAnnotatedShader);

    return ret;
}

inline bool GlSlProgram::Link()
{
    glLinkProgram(prog);
    return IsLinkSuccessPrintLog(prog);
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
    GLint handle = glGetAttribLocation(prog, name.c_str());
    if(handle == -1) std::cerr << "Attribute name doesn't exist for program (" << name << ")" << std::endl;
    return handle;
}

inline GLint GlSlProgram::GetUniformHandle(const std::string& name)
{
    GLint handle = glGetUniformLocation(prog, name.c_str());
    if(handle == -1) std::cerr << "Uniform name doesn't exist for program (" << name << ")" << std::endl;
    return handle;
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

inline void GlSlProgram::SetUniform(const std::string& name, double f)
{
    glUniform1d( GetUniformHandle(name), f);
}

inline void GlSlProgram::SetUniform(const std::string& name, double f1, double f2)
{
    glUniform2d( GetUniformHandle(name), f1,f2);
}

inline void GlSlProgram::SetUniform(const std::string& name, double f1, double f2, double f3)
{
    glUniform3d( GetUniformHandle(name), f1,f2,f3);
}

inline void GlSlProgram::SetUniform(const std::string& name, double f1, double f2, double f3, double f4)
{
    glUniform4d( GetUniformHandle(name), f1,f2,f3,f4);
}

inline void GlSlProgram::SetUniform(const std::string& name, Colour c)
{
    glUniform4f( GetUniformHandle(name), c.r, c.g, c.b, c.a);
}

inline void GlSlProgram::SetUniform(const std::string& name, const OpenGlMatrix& mat)
{
    // glUniformMatrix4dv seems to be crashing...
    float m[16];
    for (int i = 0; i < 16; ++i) {
        m[i] = (float)mat.m[i];
    }
    glUniformMatrix4fv( GetUniformHandle(name), 1, GL_FALSE, m);
}

#ifdef HAVE_EIGEN
inline void GlSlProgram::SetUniform(const std::string& name, const Eigen::Vector2f& v)
{
    glUniform2f( GetUniformHandle(name), v[0], v[1]);
}
inline void GlSlProgram::SetUniform(const std::string& name, const Eigen::Vector3f& v)
{
    glUniform3f( GetUniformHandle(name), v[0], v[1], v[2]);
}
inline void GlSlProgram::SetUniform(const std::string& name, const Eigen::Vector4f& v)
{
    glUniform4f( GetUniformHandle(name), v[0], v[1], v[2], v[3]);
}
inline void GlSlProgram::SetUniform(const std::string& name, const Eigen::Matrix2f& m)
{
    glUniformMatrix2fv( GetUniformHandle(name), 1, GL_FALSE, m.data());
}
inline void GlSlProgram::SetUniform(const std::string& name, const Eigen::Matrix3f& m)
{
    glUniformMatrix3fv( GetUniformHandle(name), 1, GL_FALSE, m.data());
}
inline void GlSlProgram::SetUniform(const std::string& name, const Eigen::Matrix4f& m)
{
    glUniformMatrix4fv( GetUniformHandle(name), 1, GL_FALSE, m.data());
}

inline void GlSlProgram::SetUniform(const std::string& name, const Eigen::Vector2d& v)
{
    glUniform2d( GetUniformHandle(name), v[0], v[1]);
}
inline void GlSlProgram::SetUniform(const std::string& name, const Eigen::Vector3d& v)
{
    glUniform3d( GetUniformHandle(name), v[0], v[1], v[2]);
}
inline void GlSlProgram::SetUniform(const std::string& name, const Eigen::Vector4d& v)
{
    glUniform4d( GetUniformHandle(name), v[0], v[1], v[2], v[3]);
}
inline void GlSlProgram::SetUniform(const std::string& name, const Eigen::Matrix2d& m)
{
    glUniformMatrix2dv( GetUniformHandle(name), 1, GL_FALSE, m.data());
}
inline void GlSlProgram::SetUniform(const std::string& name, const Eigen::Matrix3d& m)
{
    glUniformMatrix3dv( GetUniformHandle(name), 1, GL_FALSE, m.data());
}
inline void GlSlProgram::SetUniform(const std::string& name, const Eigen::Matrix4d& m)
{
    glUniformMatrix4dv( GetUniformHandle(name), 1, GL_FALSE, m.data());
}
#endif

inline void GlSlProgram::BindPangolinDefaultAttribLocationsAndLink()
{
    glBindAttribLocation(prog, DEFAULT_LOCATION_POSITION, DEFAULT_NAME_POSITION);
    glBindAttribLocation(prog, DEFAULT_LOCATION_COLOUR,   DEFAULT_NAME_COLOUR);
    glBindAttribLocation(prog, DEFAULT_LOCATION_NORMAL,   DEFAULT_NAME_NORMAL);
    glBindAttribLocation(prog, DEFAULT_LOCATION_TEXCOORD, DEFAULT_NAME_TEXCOORD);
    Link();
}

#if GL_VERSION_4_3
inline GLint GlSlProgram::GetProgramResourceIndex(const std::string& name)
{
    return glGetProgramResourceIndex(prog, GL_SHADER_STORAGE_BLOCK, name.c_str());
}

inline void GlSlProgram::SetShaderStorageBlock(const std::string& name, const int& bindingIndex)
{
    glShaderStorageBlockBinding(prog, GetProgramResourceIndex(name), bindingIndex);
}
#endif

}
