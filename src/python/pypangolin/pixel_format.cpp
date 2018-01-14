
//
// Copyright (c) Andrey Mnatsakanov
//

#include "pixel_format.hpp"
#include <pangolin/image/pixel_format.h>

namespace py_pangolin {

  void bind_pixel_format(pybind11::module& m){
    pybind11::class_<pangolin::PixelFormat>(m, "PixelFormat")
      .def_readwrite("format", &pangolin::PixelFormat::format)
      .def_readwrite("channels", &pangolin::PixelFormat::channels)
      //      .def_readwrite("channel_bits", &pangolin::PixelFormat::channel_bits) //channel_bits[4] 
      .def_readwrite("bpp", &pangolin::PixelFormat::bpp)
      .def_readwrite("planar", &pangolin::PixelFormat::planar);

    m.def("PixelFormatFromString", &pangolin::PixelFormatFromString);
  }
}  // py_pangolin
