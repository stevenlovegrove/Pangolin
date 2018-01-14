
//
// Copyright (c) Andrey Mnatsakanov
//

#include "widget.hpp"
#include <pangolin/display/widgets/widgets.h>

namespace py_pangolin {

  void bind_widget(pybind11::module &m){
    m.def("CreatePanel", &pangolin::CreatePanel, pybind11::return_value_policy::reference);
  }
}  // py_pangolin
