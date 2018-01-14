
//
// Copyright (c) Andrey Mnatsakanov
//

#include "attach.hpp"
#include <pangolin/gl/glinclude.h>
#include <pangolin/display/attach.h>

namespace py_pangolin {
  void bind_attach(pybind11::module &m){
    pybind11::enum_<pangolin::Unit>(m, "Unit")
      .value("Fraction", pangolin::Unit::Fraction)
      .value("Pixel", pangolin::Unit::Pixel)
      .value("ReversePixel", pangolin::Unit::ReversePixel)
      .export_values();

    pybind11::class_<pangolin::Attach>(m, "Attach")
      .def(pybind11::init<>())
      .def(pybind11::init<pangolin::Unit, GLfloat>())
      .def(pybind11::init<GLfloat>())
      .def_static("Pix", &pangolin::Attach::Pix)
      .def_static("ReversePix", &pangolin::Attach::ReversePix)
      .def_static("Frac", &pangolin::Attach::Frac);
  }
}
