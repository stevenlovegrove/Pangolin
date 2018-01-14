
//
// Copyright (c) Andrey Mnatsakanov
//

#include "params.hpp"
#include <pangolin/utils/params.h>

namespace py_pangolin {

    void bind_params(pybind11::module &m) {
      pybind11::class_<pangolin::Params>(m, "Params")
        .def(pybind11::init<>())
        .def("Contains", &pangolin::Params::Contains)
        .def("Set", &pangolin::Params::Set<int>)
        .def("Get", &pangolin::Params::Get<int>);
    }

}  // py_pangolin
