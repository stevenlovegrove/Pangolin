
//
// Copyright (c) Andrey Mnatsakanov
//

#include "window.hpp"
#include <pangolin/display/window.h>


namespace py_pangolin {

  class PyWindowInterface: public pangolin::WindowInterface{
    public:
    using pangolin::WindowInterface::WindowInterface;

    void ToggleFullscreen() override {
      PYBIND11_OVERLOAD_PURE(
                             void,
                             pangolin::WindowInterface,
                             ToggleFullscreen);
    }

    void Move(int x, int y) override {
      PYBIND11_OVERLOAD_PURE(
                             void,
                             pangolin::WindowInterface,
                             Move,
                             x,
                             y);
    }

    void Resize(unsigned int w, unsigned int h) override {
      PYBIND11_OVERLOAD_PURE(
                             void,
                             pangolin::WindowInterface,
                             Resize,
                             w,
                             h);
    }

    void MakeCurrent() override {
      PYBIND11_OVERLOAD_PURE(
                             void,
                             pangolin::WindowInterface,
                             MakeCurrent);
    }

    void ProcessEvents() override {
      PYBIND11_OVERLOAD_PURE(
                             void,
                             pangolin::WindowInterface,
                             ProcessEvents);
    }


    void SwapBuffers() override {
      PYBIND11_OVERLOAD_PURE(
                             void,
                             pangolin::WindowInterface,
                             SwapBuffers);
    }

  };
  
  void bind_window(pybind11::module &m) {
    pybind11::class_<pangolin::WindowInterface, PyWindowInterface > windows_interface(m, "WindowsInterface");
    windows_interface
      .def(pybind11::init<>())
      .def("ToggleFullscreen", &pangolin::WindowInterface::ToggleFullscreen)
      .def("Move", &pangolin::WindowInterface::Move)
      .def("Resize", &pangolin::WindowInterface::Resize)
      .def("MakeCurrent", &pangolin::WindowInterface::MakeCurrent)
      .def("ProcessEvents", &pangolin::WindowInterface::ProcessEvents)
      .def("SwapBuffers", &pangolin::WindowInterface::SwapBuffers);
  }
}
