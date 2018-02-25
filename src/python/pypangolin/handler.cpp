
//
// Copyright (c) Andrey Mnatsakanov
//

#include "handler.hpp"
#include <pangolin/handler/handler.h>
#include <pangolin/display/opengl_render_state.h>
#include <pangolin/plot/plotter.h>


namespace py_pangolin {
  
  void bind_handler(pybind11::module &m) {
    
    pybind11::class_<pangolin::Handler, PyHandler> handler(m, "Handler");
    handler
      .def(pybind11::init<>())
      .def("Keyboard", &pangolin::Handler::Keyboard)
      .def("Mouse", &pangolin::Handler::Mouse)
      .def("MouseMotion", &pangolin::Handler::MouseMotion)
      .def("PassiveMouseMotion", &pangolin::Handler::PassiveMouseMotion)
      .def("Special", &pangolin::Handler::Special);

    pybind11::class_<pangolin::Handler3D>(m, "Handler3D", handler)
      .def(pybind11::init<pangolin::OpenGlRenderState&, pangolin::AxisDirection, float, float>(), pybind11::arg("state"), pybind11::arg("enforce_up") = pangolin::AxisNone, pybind11::arg("trans_scale")=0.01f, pybind11::arg("zoom_factor")=PANGO_DFLT_HANDLER3D_ZF)
      .def("ValidWinDepth", &pangolin::Handler3D::ValidWinDepth)
      .def("PixelUnproject", &pangolin::Handler3D::PixelUnproject)
      .def("GetPosNormal", &pangolin::Handler3D::GetPosNormal)
      .def("Keyboard", &pangolin::Handler3D::Keyboard)
      .def("Mouse", &pangolin::Handler3D::Mouse)
      .def("MouseMotion", &pangolin::Handler3D::MouseMotion)
      .def("Special", &pangolin::Handler3D::Special);

  }
}  // py_pangolin
