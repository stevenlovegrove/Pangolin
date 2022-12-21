#pragma once

#include <pangolin/gl/glsl_program.h>

namespace pangolin
{

void printSourceForUser(
    GlSlProgram::Source const& source, int focus_line, int focus_radius);

void populateCodeFromFile(
    GlSlProgram::Source& source, GlSlProgram::PathList const& search_path,
    std::filesystem::path const& current_path);

void preprocessGlSl(
    std::istream& input, std::ostream& output,
    GlSlProgram::Defines const& program_defines,
    GlSlProgram::PathList const& search_path,
    std::filesystem::path const& current_path);

// Split 'code' into several code blocks per shader type
// shader blocks in 'code' must be annotated with:
// @start vertex, @start fragment, @start geometry or @start compute
std::vector<GlSlProgram::Source> splitAnnotatedShaders(
    GlSlProgram::Source const& soure);

}  // namespace pangolin
