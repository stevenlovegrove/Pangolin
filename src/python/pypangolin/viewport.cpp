
//
// Copyright (c) Andrey Mnatsakanov
//

#include "viewport.hpp"
#include <pangolin/display/viewport.h>

namespace py_pangolin {

  void bind_viewport(pybind11::module &m) {
    pybind11::class_<pangolin::Viewport>(m, "Viewport")
      .def(pybind11::init<>())
      .def(pybind11::init<int, int, int, int>())
      .def("Activate", &pangolin::Viewport::Activate)
      .def("ActivateIdentity", &pangolin::Viewport::ActivateIdentity)
      .def("ActivatePixelOrthographic", &pangolin::Viewport::ActivatePixelOrthographic)
      .def("Scissor", &pangolin::Viewport::Scissor)
      .def("ActivateAndScissor", &pangolin::Viewport::ActivateAndScissor)
      .def("Contains", &pangolin::Viewport::Contains)
      .def("Inset", (pangolin::Viewport (pangolin::Viewport::*)(int)const)&pangolin::Viewport::Inset)
      .def("Inset", (pangolin::Viewport (pangolin::Viewport::*)(int, int)const)&pangolin::Viewport::Inset)
      .def("Insetsect", &pangolin::Viewport::Intersect)
      .def("DisableScissor", &pangolin::Viewport::DisableScissor)
      .def("r", &pangolin::Viewport::r)
      .def("t", &pangolin::Viewport::t)
      .def("aspect", &pangolin::Viewport::aspect);
  }

}  // py_pangolin

