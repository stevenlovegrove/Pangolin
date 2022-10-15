/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) Andrey Mnatsakanov
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

#include "glfont.hpp"
#include <pangolin/gl/glfont.h>
#include <pangolin/display/default_font.h>
#include <pybind11/eigen.h>
#include <pybind11/stl.h>

namespace py_pangolin {

  void bind_glfont(pybind11::module &m){

    pybind11::class_<pangolin::GlFont> glFontClass(m, "GlFont");
    glFontClass.def(pybind11::init<std::string&, float, int, int>())
      .def("Text", (pangolin::GlText (pangolin::GlFont::*)(const std::string&))&pangolin::GlFont::Text)
      .def("Height", &pangolin::GlFont::Height)
      .def("MaxWidth", &pangolin::GlFont::MaxWidth)
      .def_static("DefaultFont", &pangolin::default_font, pybind11::return_value_policy::reference);
  }


}  // py_pangolin
