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

#include "gl_draw.hpp"
#include <pangolin/gl/gldraw.h>

namespace py_pangolin {

  void bind_gl_draw(pybind11::module &m){

    m.def("glDrawAxis",
          (void (*)(float))&pangolin::glDrawAxis);

    m.def("glDrawColouredCube",
          &pangolin::glDrawColouredCube,
          pybind11::arg("axis_min") = -0.5f,
          pybind11::arg("axis_max") = +0.5f);

    m.def("glDraw_x0",
          &pangolin::glDraw_x0);
    m.def("glDraw_y0",
          &pangolin::glDraw_y0);
    m.def("glDraw_z0",
          &pangolin::glDraw_z0);


  }


}  // py_pangolin
