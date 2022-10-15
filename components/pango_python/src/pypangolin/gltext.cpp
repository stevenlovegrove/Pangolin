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

#include "gltext.hpp"
#include <pangolin/gl/gltext.h>
#include <pybind11/eigen.h>
#include <pybind11/stl.h>

namespace py_pangolin {

  void bind_gltext(pybind11::module &m){

    pybind11::class_<pangolin::GlText> glTextClass(m, "GlText");
    glTextClass.def(pybind11::init<>())
      .def("AddSpace", &pangolin::GlText::AddSpace)
      .def("Add", &pangolin::GlText::Add)
      .def("Clear", &pangolin::GlText::Clear)
      .def("Draw", (void (pangolin::GlText::*)() const) &pangolin::GlText::Draw)
      .def("DrawGlSl", &pangolin::GlText::DrawGlSl)
      .def("Draw", (void (pangolin::GlText::*)(GLfloat, GLfloat, GLfloat) const) &pangolin::GlText::Draw, pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("z") = 0)
      .def("DrawWindow", &pangolin::GlText::DrawWindow)
      .def("Text", &pangolin::GlText::Text)
      .def("Width", &pangolin::GlText::Width)
      .def("Height", &pangolin::GlText::Height)
      .def("FullHeight", &pangolin::GlText::FullHeight);
  }


}  // py_pangolin
