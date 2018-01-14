
//
// Copyright (c) Andrey Mnatsakanov
//

#include "handler.hpp"
#include <pangolin/handler/handler.h>
#include <pangolin/display/opengl_render_state.h>
#include <pangolin/plot/plotter.h>


namespace py_pangolin {
  
  void bind_handler(pybind11::module &m) {
    
    pybind11::class_<pangolin::Handler, PyHandler> handler(m, "Handler");
    handler
      .def(pybind11::init<>())
      .def("Keyboard", &pangolin::Handler::Keyboard)
      .def("Mouse", &pangolin::Handler::Mouse)
      .def("MouseMotion", &pangolin::Handler::MouseMotion)
      .def("PassiveMouseMotion", &pangolin::Handler::PassiveMouseMotion)
      .def("Special", &pangolin::Handler::Special);

    pybind11::class_<pangolin::Handler3D>(m, "Handler3D", handler)
      .def(pybind11::init<pangolin::OpenGlRenderState&, pangolin::AxisDirection, float, float>(), pybind11::arg("state"), pybind11::arg("enforce_up") = pangolin::AxisNone, pybind11::arg("trans_scale")=0.01f, pybind11::arg("zoom_factor")=PANGO_DFLT_HANDLER3D_ZF)
      .def("ValidWinDepth", &pangolin::Handler3D::ValidWinDepth)
      .def("PixelUnproject", &pangolin::Handler3D::PixelUnproject)
      .def("GetPosNormal", &pangolin::Handler3D::GetPosNormal)
      .def("Keyboard", &pangolin::Handler3D::Keyboard)
      .def("Mouse", &pangolin::Handler3D::Mouse)
      .def("MouseMotion", &pangolin::Handler3D::MouseMotion)
      .def("Special", &pangolin::Handler3D::Special);

    pybind11::class_<pangolin::Plotter>(m, "Plotter", handler)
      .def(pybind11::init<pangolin::DataLog*, float, float, float, float, float, float, pangolin::Plotter*, pangolin::Plotter*>(),
           pybind11::arg("default_log"),
           pybind11::arg("left")=0,
           pybind11::arg("right")=600,
           pybind11::arg("bottom")=-1,
           pybind11::arg("top")=1,
           pybind11::arg("tickx")=30,
           pybind11::arg("ticky")=0.5,
           pybind11::arg("linked_plotter_x")=NULL,
           pybind11::arg("linked_plotter_y")=NULL)
      .def(pybind11::init([](pangolin::DataLog* log, float left, float right, float bottom, float top, float tickx, float ticky){return new pangolin::Plotter(log, left, right, bottom, top,tickx, ticky);}),
           pybind11::arg("default_log"),
           pybind11::arg("left")=0,
           pybind11::arg("right")=600,
           pybind11::arg("bottom")=-1,
           pybind11::arg("top")=1,
           pybind11::arg("// TODO: ickx")=30,
           pybind11::arg("ticky")=0.5)

      .def("Render", &pangolin::Plotter::Render)
      .def("GetSelection", &pangolin::Plotter::GetSelection)
      .def("GetDefaultView", &pangolin::Plotter::GetDefaultView)
      .def("SetDefaultView", &pangolin::Plotter::SetDefaultView)
      .def("GetView", &pangolin::Plotter::GetView)
      .def("SetView", &pangolin::Plotter::SetView)
      .def("SetViewSmooth", &pangolin::Plotter::SetViewSmooth)
      .def("ScrollView", &pangolin::Plotter::ScrollView)
      .def("ScrollViewSmooth", &pangolin::Plotter::ScrollViewSmooth)
      .def("ScaleView", &pangolin::Plotter::ScaleView)
      .def("ScaleViewSmooth", &pangolin::Plotter::ScaleViewSmooth)
      .def("SetTicks", &pangolin::Plotter::SetTicks)
      .def("Track", &pangolin::Plotter::Track, pybind11::arg("x")="$i", pybind11::arg("y")="")
      .def("ToggleTracking", &pangolin::Plotter::ToggleTracking)
      .def("Trigger", &pangolin::Plotter::Trigger, pybind11::arg("x")="$0", pybind11::arg("edge")=-1, pybind11::arg("value")=0.0f)
      .def("ToggleTrigger", &pangolin::Plotter::ToggleTrigger)
      .def("SetBackgroundColour", &pangolin::Plotter::SetBackgroundColour)
      .def("SetAxisColour", &pangolin::Plotter::SetAxisColour)
      .def("SetTickColour", &pangolin::Plotter::SetTickColour)
      .def("Keyboard", &pangolin::Plotter::Keyboard)
      .def("Mouse", &pangolin::Plotter::Mouse)
      .def("MouseMotion", &pangolin::Plotter::MouseMotion)
      .def("PassiveMouseMotion", &pangolin::Plotter::PassiveMouseMotion)
      .def("Special", &pangolin::Plotter::Special)
      .def("ClearSeries", &pangolin::Plotter::ClearSeries)
      .def("AddSeries", &pangolin::Plotter::AddSeries)
      .def("PlotTitleFromExpr", &pangolin::Plotter::PlotTitleFromExpr)
      .def("ClearMarkers", &pangolin::Plotter::ClearMarkers)
      .def("AddMarker", (pangolin::Marker& (pangolin::Plotter::*)(pangolin::Marker::Direction, float, pangolin::Marker::Equality, pangolin::Colour))&pangolin::Plotter::AddMarker, pybind11::arg("d"), pybind11::arg("value"), pybind11::arg("leg") = pangolin::Marker::Equality::Equal, pybind11::arg("c")=pangolin::Colour())
      .def("AddMarker", (pangolin::Marker& (pangolin::Plotter::*)(const pangolin::Marker&))&pangolin::Plotter::AddMarker)
    //      .def("ClearImplicitPlots", &pangolin::Plotter::ClearImplicitPlots);
      //   .def("AddImplicitPlot", &pangolin::Plotter::AddImplicitPlot);
      .def("SetBounds", (pangolin::View& (pangolin::Plotter::*)(pangolin::Attach, pangolin::Attach, pangolin::Attach, pangolin::Attach))&pangolin::Plotter::SetBounds)
      .def("SetBounds", (pangolin::View& (pangolin::Plotter::*)(pangolin::Attach, pangolin::Attach, pangolin::Attach, pangolin::Attach, bool))&pangolin::Plotter::SetBounds)
      .def("SetBounds", (pangolin::View& (pangolin::Plotter::*)(pangolin::Attach, pangolin::Attach, pangolin::Attach, pangolin::Attach, double))&pangolin::Plotter::SetBounds)
      .def("AddToDisplay", [](pangolin::Plotter& p, pangolin::View& v)->pangolin::View&{return v.AddDisplay(p);}, pybind11::return_value_policy::reference);
     
  }
}  // py_pangolin
