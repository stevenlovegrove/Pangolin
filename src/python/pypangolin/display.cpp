
//
// Copyright (c) Andrey Mnatsakanov
//

#include "display.hpp"
#include <pangolin/display/display.h>
#include <pangolin/display/view.h>
#include <pybind11/functional.h>

namespace py_pangolin {
  
  void bind_display(pybind11::module &m) {
    m.def("CreateWindowAndBind",
          [](const std::string& title, int w=640, int h=480) -> pangolin::WindowInterface& {
            return pangolin::CreateWindowAndBind(title, w, h);
          },
          pybind11::return_value_policy::reference,
          pybind11::arg("window_title"),
          pybind11::arg("w") = 640,
          pybind11::arg("h") = 480);

    m.def("CreateWindowAndBindWithParams",
          &pangolin::CreateWindowAndBind,
          pybind11::return_value_policy::reference);

    m.def("CreateDisplay",
          &pangolin::CreateDisplay,
          pybind11::return_value_policy::reference);

    m.def("ShouldQuit",
          &pangolin::ShouldQuit);

    m.def("FinishFrame",
          &pangolin::FinishFrame);

    m.def("ToggleFullscreen",
          &pangolin::ToggleFullscreen);

    m.def("ToggleConsole",
          &pangolin::ToggleConsole);

    m.def("RegisterKeyPressCallback",
          [](int v, const std::function<void()>& f){
            pangolin::RegisterKeyPressCallback(v, f);
          }
    );
    
    m.def("DisplayBase",
          &pangolin::DisplayBase,
          pybind11::return_value_policy::reference);

    m.def("Display",
          &pangolin::Display,
          pybind11::return_value_policy::reference,
          pybind11::arg("name"));
  }

}  // Py_pangolin
