//
// Copyright (c) Andrey Mnatsakanov
//

#ifndef PY_PANGOLIN_IMAGE_HPP
#define PY_PANGOLIN_IMAGE_HPP

#include <pybind11/pybind11.h>
#include <pangolin/image/image.h>

namespace py_pangolin {

  template<typename T>
  void bind_image(pybind11::module& m, const std::string& pyname){
    pybind11::class_<pangolin::Image<T> >(m, pyname.c_str())
      .def(pybind11::init<>())
      .def(pybind11::init<T*, size_t, size_t, size_t>())
      .def("SizeBytes", &pangolin::Image<T>::SizeBytes)
      .def("Area", &pangolin::Image<T>::Area)
      .def("IsValid", &pangolin::Image<T>::IsValid)
      .def("IsContiguous", &pangolin::Image<T>::IsContiguous)
      .def("begin", (unsigned char* (pangolin::Image<T>::*)())&pangolin::Image<T>::begin)
      .def("end", (unsigned char* (pangolin::Image<T>::*)())&pangolin::Image<T>::end)
      .def("begin", (const unsigned char* (pangolin::Image<T>::*)() const )&pangolin::Image<T>::begin)
      .def("end", (const unsigned char* (pangolin::Image<T>::*)() const )&pangolin::Image<T>::end)
      .def("size", &pangolin::Image<T>::size)
      .def("Fill", &pangolin::Image<T>::Fill)
      .def("Replace", &pangolin::Image<T>::Replace)
      .def("Memset", &pangolin::Image<T>::Memset)
      .def("CopyFrom", &pangolin::Image<T>::CopyFrom)
      .def("MinMax", &pangolin::Image<T>::MinMax)
      // .def("Sum", [](pangolin::Image<T>& v){ return v.Sum();})
      // .def("Mean", [](pangolin::Image<T>& v){ return v.Mean();})
      .def("RowPtr", (T* (pangolin::Image<T>::*)(size_t))&pangolin::Image<T>::RowPtr)
      .def("RowPtr", (const T* (pangolin::Image<T>::*)(size_t) const )&pangolin::Image<T>::RowPtr)
      .def("InImage", &pangolin::Image<T>::InImage)
      .def("InBounds", (bool (pangolin::Image<T>::*)(int, int) const)&pangolin::Image<T>::InBounds)
      .def("InBounds", (bool (pangolin::Image<T>::*)(float, float, float) const)&pangolin::Image<T>::InBounds)
      // .def("SubImage", (pangolin::Image<T> (pangolin::Image<T>::*)(size_t, size_t, size_t, size_t))&pangolin::Image<T>::SubImage)
      // .def("Row", &pangolin::Image<T>::Row)
      // .def("Col", &pangolin::Image<T>::Col)
      .def("InImage", &pangolin::Image<T>::InImage);
  }

}  // py_pangolin

#endif
