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

#pragma once

#include <pangolin/gl/uniform.h>
#include <pangolin/utils/scoped_bind.h>
#include <pangolin/utils/shared.h>

#include <filesystem>
#include <map>
#include <vector>

namespace pangolin
{

enum class ShaderType {
  Annotated = 0,      // Source is annotated with possibly multiple
                      // shaders @start fragment, @start vertex,
                      // @start geometry, or @start compute
                      //
  Fragment = 0x8B30,  // GL_FRAGMENT_SHADER
  Vertex = 0x8B31,    // GL_VERTEX_SHADER
  Geometry = 0x8DD9,  // GL_GEOMETRY_SHADER
  Compute = 0x91B9,   // GL_COMPUTE_SHADE
};

struct GlSlProgram {
  struct LinkDetails {
    uint32_t datatype;     // e.g. GL_FLOAT, GL_FLOAT_VEC2, GL_FLOAT_VEC3
    int32_t num_elements;  // not entirely sure yet...
    int32_t location;      // index that should be used for binding
  };
  using Defines = std::map<std::string, std::string>;
  using PathList = std::vector<std::filesystem::path>;
  using Attributes = std::map<std::string, LinkDetails>;
  using Uniforms = std::map<std::string, LinkDetails>;

  // Definition of a shader to add to the program.
  // The shader can be specified via the path 'origin', or
  // if glsl_code is not empty, it will be used instead. In
  // such a case, the origin and origin_line will be used
  // for compile error messages only.
  struct Source {
    ShaderType shader_type = ShaderType::Annotated;
    std::filesystem::path origin;
    std::string glsl_code = "";
    int origin_line = 0;
  };

  virtual ~GlSlProgram() {}

  virtual ScopedBind<GlSlProgram> bind() const = 0;

  virtual Attributes getAttributes() const = 0;

  virtual Uniforms getUniforms() const = 0;

  virtual void reload() = 0;

  struct Params {
    std::vector<Source> sources;
    Defines program_defines = {};
    PathList search_path = {};
    bool link_immediately = true;
    bool automatically_reload = true;
  };
  static Shared<GlSlProgram> Create(Params p);
};

}  // namespace pangolin
