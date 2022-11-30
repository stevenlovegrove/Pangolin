#include <pangolin/gl/glsl_preprocessor.h>
#include <pangolin/utils/string.h>
#include <unordered_map>
#include <fstream>
#include <fmt/ranges.h>

namespace pangolin
{

// forward declaration of map build by cmake (into shaders.cpp)
const std::unordered_map<std::string,const char*>& GetBuiltinShaders();

constexpr const char* PANGO_BEGIN_MARKER = "// PANGO_BEGIN_INCLUDE ";
constexpr const char* PANGO_END_MARKER = "// PANGO_END_INCLUDE ";

void includeBeginMarker(std::ostream& output, const std::filesystem::path& full_include_path)
{
    output << PANGO_BEGIN_MARKER << full_include_path << std::endl;
}

void includeEndMarker(std::ostream& output, const std::filesystem::path& full_include_path)
{
    output << PANGO_END_MARKER << full_include_path << std::endl;
}

// focus_line is a line to highlight
// focus_radius is the number of lines to show around the focus line
// focus feature is not used if focus_line < 0, in which case all code is
// shown
void printSourceForUser(
    const GlSlProgram::Source& source,
    int focus_line,
    int focus_radius)
{
    std::istringstream is(source.glsl_code);

    std::string line;
    int preprocessed_line_num = 0;

    std::vector<std::string> file_stack = {source.origin};
    std::vector<int> line_stack = {source.origin_line};

    std::string error_file;
    int error_line = 0;

    if(focus_line >=0 && focus_line - focus_radius > 0) {
        fmt::print(fmt::emphasis::faint, "...\n");
    }

    while (std::getline(is, line)) {
        ++preprocessed_line_num;

        if(startsWith(line, PANGO_BEGIN_MARKER)) {
            file_stack.push_back( line.substr(strlen(PANGO_BEGIN_MARKER)) );
            line_stack.push_back(0);
        }else if( startsWith(line, PANGO_END_MARKER)) {
            file_stack.pop_back();
            line_stack.pop_back();
            // Should always have at least the first element we added above
            PANGO_ENSURE(file_stack.size() > 0 && line_stack.size() > 0);
        }

        if( focus_line == -1 || std::abs(preprocessed_line_num - focus_line) < focus_radius) {
            if(preprocessed_line_num == focus_line) {
                error_file = file_stack.back();
                error_line = line_stack.back();
                fmt::print(fg(fmt::color::crimson), "{}\n", line);
            }else{
                fmt::print(fmt::emphasis::faint, "{}\n", line);
            }
        }
        ++line_stack.back();
    }

    if(focus_line >= 0) {
        fmt::print(fmt::emphasis::faint, "...\n");
        println(fg(fmt::color::crimson), "@ {}:{}", error_file, error_line);
    }
}

std::string filenameFromIncludeToken(const std::string& location)
{
    size_t start = location.find_first_of("\"<");
    if(start != std::string::npos) {
        size_t end = location.find_first_of("\">", start+1);
        if(end != std::string::npos) {
            return location.substr(start+1, end - start - 1);
        }
    }
    PANGO_THROW("GLSL Parser: Unable to parse include location '{}'", location );
    PANGO_UNREACHABLE();
}

bool builtInPathExists(const std::filesystem::path& filename)
{
    auto it = GetBuiltinShaders().find(filename.string());
    return it != GetBuiltinShaders().end();
}

// Version of pangolin::FileExists that also looks in the builtin_shaders map
bool builtInOrRealPathExists(const std::filesystem::path& filename)
{
    return builtInPathExists(filename) || std::filesystem::exists(filename);
}

std::optional<std::filesystem::path> findInPath(
    const std::filesystem::path& include_path,
    const std::vector<std::filesystem::path>& search_path,
    const std::filesystem::path& current_path,
    const std::function<bool(const std::filesystem::path&)>& exists_func
) {
    if( exists_func(include_path)) {
        return include_path;
    }else if( exists_func(current_path / include_path)) {
        return current_path / include_path;
    }else{
        for(auto path : search_path) {
            if( exists_func(path / include_path) )
                return path / include_path;
        }
    }
    return std::nullopt;
}

struct Directive {
    enum class Kind { include, expect };
    Kind kind;
    std::vector<std::string> tokens;
};

std::optional<std::vector<std::string>> maybeTokenized(const std::string& line, char escape_char)
{
    const size_t hash = line.find(escape_char);
    if( hash != std::string::npos ) {
        for(size_t i=0; i < hash; ++i) {
            // only tokenize if escape_car is first non-whitespace charector.
            if(!std::isspace(line[hash])) return std::nullopt;
        }

        std::istringstream iss(std::string(line.begin() + hash+1, line.end()));
        return tokenize(iss);
    }
    return std::nullopt;
}

std::optional<Directive> getDirective(const std::string& line)
{
    if(auto maybe_tokens = maybeTokenized(line, '#')) {
        const std::vector<std::string>& tokens = *maybe_tokens;
        PANGO_THROW_IF(tokens.size()==0, "Stray '#' in GLSL line: {}", line);

        const std::string dstr = lowercased(tokens[0]);
        if( dstr == "include") {
            return Directive { .kind = Directive::Kind::include, .tokens = tokens };
        }else if( dstr == "expect") {
            return Directive { .kind = Directive::Kind::expect, .tokens = tokens };
        }else if( dstr == "version" || dstr == "define") {
            // The GlSl compiler will use these - we can ignore them.
            return std::nullopt;
        }else{
            PANGO_WARN("Unknown directive in GLSL line: {}", line);
        }
    }
    return std::nullopt;
}

void preprocessInclude(
    const std::filesystem::path& include_path,
    std::ostream& output,
    const GlSlProgram::Defines& program_defines,
    const GlSlProgram::PathList &search_path,
    const std::filesystem::path &current_path
) {
    auto maybe_file = findInPath(include_path, search_path, current_path, builtInOrRealPathExists);
    PANGO_THROW_IF(!maybe_file, "Cannot find include '{}' on search path.\nCurrent Directory: {}\nSearch Path: [{}].", include_path, current_path, search_path);
    const std::filesystem::path resolved_path = *maybe_file;

    auto it = GetBuiltinShaders().find(resolved_path.string());
    if(it != GetBuiltinShaders().end()) {
        std::istringstream stream(it->second);
        preprocessGlSl(stream, output, program_defines, search_path, resolved_path.parent_path());
    }else{
        std::ifstream stream(resolved_path);
        PANGO_THROW_IF(!stream.good(), "Include '{}' exists, but can't be opened.", resolved_path);
        preprocessGlSl(stream, output, program_defines, search_path, resolved_path.parent_path());
    }
}

void preprocessGlSl(
    std::istream& input, std::ostream& output,
    const GlSlProgram::Defines& program_defines,
    const GlSlProgram::PathList &search_path,
    const std::filesystem::path &current_path
) {
    std::string line;

    while(!input.eof()) {
        // Take like from source
        std::getline(input, line);

        // Check if there is a preprocessor directive
        auto maybe_directive = getDirective(line);
        if(!maybe_directive) {
            output << line << std::endl;
            continue;
        }

        switch(maybe_directive->kind) {
        case Directive::Kind::include:
        {
            // C++ / G3D style include directive
            PANGO_THROW_IF(maybe_directive->tokens.size() != 2);

            const std::string full_include_path = filenameFromIncludeToken(maybe_directive->tokens[1]);
            includeBeginMarker(output, full_include_path);
            preprocessInclude( full_include_path, output, program_defines, search_path, current_path );
            includeEndMarker(output, full_include_path);
            continue;
        }
        case Directive::Kind::expect:
        {
            // G3D style 'expect' directive, annotating expected preprocessor
            // definition with document string
            PANGO_THROW_IF(maybe_directive->tokens.size() != 2);
            const std::string token = maybe_directive->tokens[1];

            std::map<std::string,std::string>::const_iterator it = program_defines.find(token);
            if( it == program_defines.end() ) {
                PANGO_WARN("#expect define '{}' missing. Defaulting to 0.", token );
                output << "#define " << token << " 0" << std::endl;
            }else{
                output << "#define " << token << " " << it->second << std::endl;
            }
            continue;
        }
        default:
            PANGO_UNREACHABLE();
        }
    }
}

void populateCodeFromFile(
    GlSlProgram::Source& source,
    const GlSlProgram::PathList &search_path,
    const std::filesystem::path &current_path
) {
    //
    PANGO_ENSURE(!source.origin.empty() || !source.glsl_code.empty(),
        "Need at least one of origin or glsl_code to be specified");

    if(!source.origin.empty()) {
        if (!source.glsl_code.empty()) {
            PANGO_INFO("Reloading shader source '{}'", source.origin);
        }
    }else{
        // glsl_code is populated already
        PANGO_ASSERT(!source.glsl_code.empty());
        return;
    }

    auto maybe_file = findInPath(source.origin, search_path, current_path, builtInOrRealPathExists);
    PANGO_THROW_IF(!maybe_file, "Cannot find root sourcefile {} on search path.\nCurrent Directory: {}\nSearch Path: {}.", source.origin, current_path, search_path);
    const std::filesystem::path resolved_path = *maybe_file;

    auto it = GetBuiltinShaders().find(resolved_path.string());
    if(it != GetBuiltinShaders().end()) {
        source.glsl_code = it->second;
    }else{
        std::ifstream file(resolved_path);
        PANGO_THROW_IF(!file.good(), "Include '{}' exists, but can't be opened.", resolved_path);
        std::stringstream buffer;
        buffer << file.rdbuf();
        source.glsl_code = buffer.str();
    }
}

std::vector<GlSlProgram::Source>
splitAnnotatedShaders(const GlSlProgram::Source& source)
{
    std::vector<GlSlProgram::Source> ret;
    ShaderType current_type = source.shader_type;
    int current_start_line_num = 0;

    if(current_type != ShaderType::Annotated) {
        // return as-is. No split needed
        return {source};
    }

    int line_num = 0;

    std::stringstream output;
    auto finish_block = [&](ShaderType new_type){
        GlSlProgram::Source new_src = {
            .shader_type = current_type,
            .origin = source.origin,
            .glsl_code = output.str(),
            .origin_line = current_start_line_num
        };
        if(current_type != ShaderType::Annotated && new_src.glsl_code.size() > 0) {
            ret.push_back(new_src);
        }
        output.str(std::string());
        current_type = new_type;
        current_start_line_num = line_num;
    };

    std::stringstream input(source.glsl_code);
    std::string line;

    while(!input.eof()) {
        ++line_num;

        // Take like from source
        std::getline(input, line);

        auto maybe_tokens = maybeTokenized(line, '@');
        if(maybe_tokens) {
            const auto& tokens = *maybe_tokens;
            PANGO_THROW_IF( tokens.size() != 2, "Bad annotation in line '{}'", line );
            PANGO_THROW_IF( lowercased(tokens[0]) != "start", "Annotation must begin with 'start' token in line '{}'. Found {}", line, tokens[0] );
            const std::string str_shader_type = lowercased(tokens[1]);
            if(str_shader_type == "vertex") {
                finish_block(ShaderType::Vertex);
            }else if(str_shader_type == "fragment") {
                finish_block(ShaderType::Fragment);
            }else if(str_shader_type == "geometry") {
                finish_block(ShaderType::Geometry);
            }else if(str_shader_type == "compute") {
                finish_block(ShaderType::Compute);
            }else{
                PANGO_UNREACHABLE();
            }
        }else{
            output << line << std::endl;
        }
    }

    finish_block(ShaderType::Annotated);

    return ret;
}


}
