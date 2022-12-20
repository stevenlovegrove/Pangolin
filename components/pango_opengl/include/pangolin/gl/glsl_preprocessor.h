#pragma once

#include <pangolin/gl/glsl_program.h>

namespace pangolin
{

void printSourceForUser(
    const GlSlProgram::Source& source, int focus_line, int focus_radius);

void populateCodeFromFile(
    GlSlProgram::Source& source, const GlSlProgram::PathList& search_path,
    const std::filesystem::path& current_path);

void preprocessGlSl(
    std::istream& input, std::ostream& output,
    const GlSlProgram::Defines& program_defines,
    const GlSlProgram::PathList& search_path,
    const std::filesystem::path& current_path);

// Split 'code' into several code blocks per shader type
// shader blocks in 'code' must be annotated with:
// @start vertex, @start fragment, @start geometry or @start compute
std::vector<GlSlProgram::Source> splitAnnotatedShaders(
    const GlSlProgram::Source& soure);

}  // namespace pangolin
