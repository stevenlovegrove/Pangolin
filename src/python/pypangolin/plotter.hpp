//
// Copyright (c) Andrey Mnatsakanov
//

#ifndef PY_PANGOLIN_PLOTTER_HPP
#define PY_PANGOLIN_PLOTTER_HPP

#include <pybind11/pybind11.h>

namespace py_pangolin {

  void bind_plotter(pybind11::module& m);

}  // py_pangolin

#endif
