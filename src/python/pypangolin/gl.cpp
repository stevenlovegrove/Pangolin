
//
// Copyright (c) Andrey Mnatsakanov
//

#include "gl.hpp"
#include <pangolin/gl/glplatform.h>

namespace py_pangolin {

  void bind_gl(pybind11::module &m){
    m.def("glEnable", glEnable);
    m.def("glClear", glEnable);
    m.def("glColor3f", glColor3f);
    m.attr("GL_DEPTH_TEST") = (int32_t)GL_DEPTH_TEST;
    m.attr("GL_COLOR_BUFFER_BIT") = (int32_t)GL_COLOR_BUFFER_BIT;
    m.attr("GL_DEPTH_BUFFER_BIT") = (int32_t)GL_DEPTH_BUFFER_BIT;
    
    m.def("glEnableDepthTest", [](){glEnable(GL_DEPTH_TEST);});
    m.def("glClearColorBifferBitOrDepthBufferBit", [](){glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);});
  }

}
