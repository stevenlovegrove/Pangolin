
#include <Eigen/Core>
#include <pangolin/gl/glplatform.h>
#include <pangolin/gl/glsl_preprocessor.h>
#include <pangolin/utils/string.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <unordered_map>

namespace pangolin
{

template <>
ScopedBind<GlSlProgram>::pScopedBind&
ScopedBind<GlSlProgram>::getLocalActiveScopePtr()
{
  static thread_local pScopedBind x = nullptr;
  return x;
}

////////////////////////////////////////////////////////////////////////////////////
// Utilities

bool checkLinkSuccess(GLhandleARB prog)
{
  GLint status;
  glGetProgramiv(prog, GL_LINK_STATUS, &status);
  if (status != GL_TRUE) {
    const int PROGRAM_LOG_MAX_LEN = 10240;
    char infolog[PROGRAM_LOG_MAX_LEN];
    GLsizei len = 0;
    PANGO_GL(glGetProgramInfoLog(prog, PROGRAM_LOG_MAX_LEN, &len, infolog));
    PANGO_ERROR(
        "GLSL Program link failed:\n{}", len ? infolog : "No details provided");
    return false;
  }
  return true;
}

std::pair<int, int> lineNumFromCompilationError(const std::string& info)
{
  auto tokens = split(info, ':');
  if (tokens.size() >= 3 && tokens[0] == "ERROR") {
    int column = std::stoi(tokens[1]);
    int line = std::stoi(tokens[2]);
    return {column, line};
  } else {
    PANGO_DEBUG("Unable to parse GlSl error string to extract line number");
  }
  return {-1, -1};
}

bool checkCompileSuccess(GLhandleARB shader, const GlSlProgram::Source& source)
{
  GLint status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE) {
    const int SHADER_LOG_MAX_LEN = 10240;
    char infolog[SHADER_LOG_MAX_LEN];
    GLsizei len;
    PANGO_GL(glGetShaderInfoLog(shader, SHADER_LOG_MAX_LEN, &len, infolog));
    PANGO_ERROR(
        "GLSL Shader compilation failed in file starting at\n{}:{}\n{}",
        source.origin.string(), source.origin_line,
        len ? infolog : "No details provided");

    if (!source.glsl_code.empty()) {
      auto [col, line] = lineNumFromCompilationError(infolog);
      printSourceForUser(source, line, 5);
    }
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Implementation of GlSlProgram

struct GlSlProgramImpl : public GlSlProgram {
  GlSlProgramImpl(const GlSlProgram::Params& p) { reload(p); }

  bool reinitialize()
  {
    if (!prog_) {
      // program object
      prog_ = glCreateProgram();
      PANGO_ENSURE(prog_ > 0);
    } else {
      clearShaders();
    }

    // Perform any code transformations we have
    sources_ = processSources(source_inputs_);

    if (sources_.size() == 0) {
      PANGO_WARN("No concrete shader types found");
    }

    // compile shaders
    for (auto& s : sources_) {
      PANGO_ENSURE(s.shader_type != ShaderType::Annotated);
      GLhandleARB shader = glCreateShader(static_cast<GLenum>(s.shader_type));
      const char* src = s.glsl_code.c_str();
      glShaderSource(shader, 1, &src, nullptr);
      glCompileShader(shader);
      if (checkCompileSuccess(shader, s)) {
        glAttachShader(prog_, shader);
        shaders_.push_back(shader);
      } else {
        // invalidate entire program.
        // checkCompileSuccess will have printed diagnostic info
        clearProgram();
        return false;
      }
    }

    // link
    glLinkProgram(prog_);
    return checkLinkSuccess(prog_);
  }

  bool ensureLinked()
  {
    if (prog_) return true;
    if (!link_failed_) {
      link_failed_ = !reinitialize();
    }
    return !link_failed_;
  }

  // Transform sources with preprocessor.
  // This will split combined shaders and enable include directives etc.
  std::vector<Source> processSources(std::vector<Source> input)
  {
    std::vector<Source> new_sources;

    // Preload source from files
    for (auto& in : input) {
      populateCodeFromFile(in, search_path_, "");
    }

    // Split any annotated shaders
    for (const auto& in : input) {
      auto split = splitAnnotatedShaders(in);
      new_sources.insert(new_sources.begin(), split.begin(), split.end());
      if (split.size() == 0)
        PANGO_WARN("Shader source file '{}' contained no shaders", in.origin);
    }

    // Preprocess
    for (auto& s : new_sources) {
      std::ostringstream process_stream;
      std::filesystem::path current_path = s.origin.parent_path();

      if (s.glsl_code.empty()) {
        // load from file
        std::ifstream file_stream(s.origin);
        PANGO_THROW_IF(!file_stream.good());
        preprocessGlSl(
            file_stream, process_stream, program_defines_, search_path_,
            current_path);
      } else {
        // read from string
        std::istringstream string_stream(s.glsl_code);
        preprocessGlSl(
            string_stream, process_stream, program_defines_, search_path_,
            current_path);
      }
      // replace / set
      s.glsl_code = process_stream.str();
    }

    return new_sources;
  }

  ~GlSlProgramImpl() { clearProgram(); }

  void clearProgram()
  {
    clearShaders();
    if (prog_) {
      glDeleteProgram(prog_);
      prog_ = 0;
    }
    link_failed_ = false;
  }

  void clearShaders()
  {
    for (GLhandleARB s : shaders_) {
      PANGO_GL(glDetachShader(prog_, s));
      PANGO_GL(glDeleteShader(s));
    }
    shaders_.clear();

    // notify any connected objects.
    signal_event_(Event::program_unlinked);
  }

  sigslot::signal<Event>& signalEvent() override { return signal_event_; }

  ScopedBind<GlSlProgram> bind() override
  {
    if (ensureLinked()) {
      const GLenum prog = prog_;
      return {[prog]() { glUseProgram(prog); }, []() { glUseProgram(0); }};
    } else {
      return {[]() {}, []() {}};
    }
  }

  Attributes getAttributes() override
  {
    if (!ensureLinked()) {
      return Attributes();
    }

    GLint count = 0;
    PANGO_GL(glGetProgramiv(prog_, GL_ACTIVE_ATTRIBUTES, &count));

    Attributes ret;
    for (int i = 0; i < count; i++) {
      LinkDetails details = {};

      constexpr GLsizei kNameLength = 1024;
      GLchar name[kNameLength];
      GLsizei name_length = 0;

      glGetActiveAttrib(
          prog_, (GLuint)i, kNameLength, &name_length, &details.num_elements,
          &details.datatype, name);

      if (name_length == 0) {
        PANGO_WARN("bad return from glGetActiveAttrib");
        continue;
      }

      details.location = glGetAttribLocation(prog_, name);
      if (details.location == 0) {
        PANGO_WARN("bad return from glGetAttribLocation");
        continue;
      }

      ret[std::string(name, name_length)] = details;
    }

    return ret;
  }

  Uniforms getUniforms() override
  {
    if (!ensureLinked()) {
      return Uniforms();
    }

    GLint count = 0;
    PANGO_GL(glGetProgramiv(prog_, GL_ACTIVE_UNIFORMS, &count));

    Uniforms ret;
    for (int i = 0; i < count; i++) {
      LinkDetails details = {};

      constexpr GLsizei kNameLength = 1024;
      GLchar name[kNameLength];
      GLsizei name_length = 0;

      glGetActiveUniform(
          prog_, (GLuint)i, kNameLength, &name_length, &details.num_elements,
          &details.datatype, name);

      if (name_length == 0) {
        PANGO_WARN("bad return from glGetActiveUniform");
        continue;
      }

      details.location = glGetUniformLocation(prog_, name);
      if (details.location == 0) {
        PANGO_WARN("bad return from glGetUniformLocation");
        continue;
      }

      ret[std::string(name, name_length)] = details;
    }

    return ret;
  }

  void reload() override
  {
    clearProgram();
    if (link_immediately_) {
      reinitialize();
    }
  }

  void reload(const Params& p) override
  {
    prog_ = 0;
    program_defines_ = p.program_defines;
    search_path_ = p.search_path;
    source_inputs_ = p.sources;
    link_immediately_ = p.link_immediately;
    reload();
  }

  GLenum prog_ = 0;
  std::vector<GLhandleARB> shaders_;
  Defines program_defines_ = {};
  PathList search_path_ = {};
  bool link_immediately_ = false;
  std::vector<Source> source_inputs_;
  std::vector<Source> sources_;
  sigslot::signal<Event> signal_event_;
  bool link_failed_ = false;
};

Shared<GlSlProgram> GlSlProgram::Create(const GlSlProgram::Params& p)
{
  return Shared<GlSlProgramImpl>::make(p);
}

}  // namespace pangolin
