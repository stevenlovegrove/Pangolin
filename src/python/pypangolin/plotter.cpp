
//
// Copyright (c) Andrey Mnatsakanov
//

#include "plotter.hpp"
#include "handler.hpp"
#include <pangolin/plot/plotter.h>


namespace py_pangolin {

  //extern pybind11::class_<pangolin::Handler, PyHandler> handler;
  
  void bind_plotter(pybind11::module& m){
  //   pybind11::class_<pangolin::Handler, PyHandler> handler1(m, "Handler1");
  //   handler1
  //     .def(pybind11::init<>())
  //     .def("Keyboard", &pangolin::Handler::Keyboard)
  //     .def("Mouse", &pangolin::Handler::Mouse)
  //     .def("MouseMotion", &pangolin::Handler::MouseMotion)
  //     .def("PassiveMouseMotion", &pangolin::Handler::PassiveMouseMotion)
  //     .def("Special", &pangolin::Handler::Special);

    pybind11::enum_<pangolin::DrawingMode>(m, "DrawingMode")
      .value("DrawingModePoints", pangolin::DrawingMode::DrawingModePoints)
      .value("DrawingModeDashed", pangolin::DrawingMode::DrawingModeDashed)
      .value("DrawingModeLine", pangolin::DrawingMode::DrawingModeLine)
      .value("DrawingModeNone", pangolin::DrawingMode::DrawingModeNone)
      .export_values();

    pybind11::class_<pangolin::Marker> marker(m, "Marker");

    pybind11::enum_<pangolin::Marker::Direction>(marker, "Direction")
      .value("Horizontal", pangolin::Marker::Direction::Horizontal)
      .value("Vertical", pangolin::Marker::Direction::Vertical)
      .export_values();

    pybind11::enum_<pangolin::Marker::Equality>(marker, "Equality")
      .value("LessThan", pangolin::Marker::Equality::LessThan)
      .value("Equal", pangolin::Marker::Equality::Equal)
      .value("GreaterThan", pangolin::Marker::Equality::GreaterThan)
      .export_values();

    marker.def(pybind11::init<pangolin::Marker::Direction, float, pangolin::Marker::Equality, pangolin::Colour>(), pybind11::arg("d"), pybind11::arg("value"), pybind11::arg("leg") = pangolin::Marker::Equality::Equal, pybind11::arg("c")=pangolin::Colour())
      .def(pybind11::init<const pangolin::XYRangef&, const pangolin::Colour&>(), pybind11::arg("range"), pybind11::arg("c")=pangolin::Colour())
      .def_readwrite("range", &pangolin::Marker::range)
      .def_readwrite("colour", &pangolin::Marker::colour);    
  }

}  // py_pangolin
