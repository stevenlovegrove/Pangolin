//
// Copyright (c) Andrey Mnatsakanov
//

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


#include "attach.hpp"
#include "colour.hpp"
#include "datalog.hpp"
#include "display.hpp"
#include "gl_draw.hpp"
#include "handler.hpp"
#include "image.hpp"
#include "opengl_render_state.hpp"
#include "params.hpp"
#include "pixel_format.hpp"
#include "plotter.hpp"
#include "var.hpp"
#include "video.hpp"
#include "view.hpp"
#include "viewport.hpp"
#include "widget.hpp"
#include "window.hpp"


PYBIND11_MODULE(pypangolin, m) {
  m.doc() = "pybind11 example plugin"; // optional module docstring

  py_pangolin::bind_var(m);
  py_pangolin::bind_viewport(m);
  py_pangolin::bind_view(m);
  py_pangolin::bind_window(m);
  py_pangolin::bind_display(m);
  py_pangolin::bind_params(m);
  py_pangolin::bind_opengl_render_state(m);
  py_pangolin::bind_attach(m);
  py_pangolin::bind_colour(m);
  py_pangolin::bind_datalog(m);
  py_pangolin::bind_plotter(m);
  py_pangolin::bind_handler(m);
  py_pangolin::bind_gl_draw(m);
  py_pangolin::bind_widget(m);
  py_pangolin::bind_pixel_format(m);
  py_pangolin::bind_image<unsigned char>(m, "Image");
  py_pangolin::bind_video(m);
}
