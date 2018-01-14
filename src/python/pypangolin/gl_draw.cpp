
//
// Copyright (c) Andrey Mnatsakanov
//

#include "gl_draw.hpp"
#include <pangolin/gl/gldraw.h>

namespace py_pangolin {

  void bind_gl_draw(pybind11::module &m){
    m.def("glDrawColouredCube",
          &pangolin::glDrawColouredCube,
          pybind11::arg("axis_min") = -0.5f,
          pybind11::arg("axis_max") = +0.5f);
  }


}  // py_pangolin
