
//
// Copyright (c) Andrey Mnatsakanov
//

#include "view.hpp"
#include <pangolin/display/view.h>
#include <pangolin/display/opengl_render_state.h>
#include <pangolin/handler/handler.h>

namespace py_pangolin {

    void bind_view(pybind11::module &m) {
    pybind11::class_<pangolin::View>(m, "View")
      .def(pybind11::init<double>(), pybind11::arg("aspect") = 0.0)
      .def("Activate", (void(pangolin::View::*)() const)&pangolin::View::Activate)
      .def("Activate", (void(pangolin::View::*)(const pangolin::OpenGlRenderState&) const)&pangolin::View::Activate)
      .def("ActivateAndScissor", (void(pangolin::View::*)() const)&pangolin::View::ActivateAndScissor)
      .def("ActivateScissorAndClear", (void(pangolin::View::*)() const)&pangolin::View::ActivateScissorAndClear)
      .def("ActivateAndScissor", (void(pangolin::View::*)(const pangolin::OpenGlRenderState&) const)&pangolin::View::ActivateAndScissor)
      .def("ActivateScissorAndClear", (void(pangolin::View::*)(const pangolin::OpenGlRenderState&) const)&pangolin::View::ActivateScissorAndClear)
      .def("ActivatePixelOrthographic", &pangolin::View::ActivatePixelOrthographic)
      .def("ActivateIdentity", &pangolin::View::ActivateIdentity)
      .def("GetClosestDepth", &pangolin::View::GetClosestDepth)
      .def("GetCamCoordinates", &pangolin::View::GetCamCoordinates)
      .def("GetObjectCoordinates", &pangolin::View::GetObjectCoordinates)
      .def("Resize", &pangolin::View::Resize)
      .def("ResizeChildren", &pangolin::View::ResizeChildren)
      .def("Render", &pangolin::View::Render)
      .def("RenderChildren", &pangolin::View::RenderChildren)
      .def("SetFocus", &pangolin::View::SetFocus)
      .def("HasFocus", &pangolin::View::HasFocus)
      .def("SetBounds", (pangolin::View& (pangolin::View::*)(pangolin::Attach, pangolin::Attach, pangolin::Attach, pangolin::Attach))&pangolin::View::SetBounds)
      .def("SetBounds", (pangolin::View& (pangolin::View::*)(pangolin::Attach, pangolin::Attach, pangolin::Attach, pangolin::Attach, bool))&pangolin::View::SetBounds)
      .def("SetBounds", (pangolin::View& (pangolin::View::*)(pangolin::Attach, pangolin::Attach, pangolin::Attach, pangolin::Attach, double))&pangolin::View::SetBounds)
      .def("SetHandler", [](pangolin::View& v, pangolin::Handler& h) -> pangolin::View& {return v.SetHandler(&h);}, pybind11::return_value_policy::reference)
      .def("SetDrawFunction", &pangolin::View::SetDrawFunction)
      .def("SetAspect", &pangolin::View::SetAspect)
      .def("SetLock", &pangolin::View::SetLock)
      .def("SetLayout", &pangolin::View::SetLayout)
      .def("AddDisplay", &pangolin::View::AddDisplay)
      .def("Show", &pangolin::View::Show, pybind11::arg("show") = true)
      .def("ToggleShow", &pangolin::View::ToggleShow)
      .def("IsShown", &pangolin::View::IsShown)
      .def("GetBounds", &pangolin::View::GetBounds)
      .def("SaveOnRender", &pangolin::View::SaveOnRender)
      .def("RecordOnRender", &pangolin::View::RecordOnRender)
      .def("SaveRenderNow", &pangolin::View::SaveRenderNow)
      .def("NumChildren", &pangolin::View::NumChildren)
      .def("GetChild", [] (pangolin::View &v, size_t i) -> pangolin::View& { return v[i];})
      .def("VisibleChild", &pangolin::View::VisibleChild)
      .def("FindChild", &pangolin::View::FindChild)
      .def("NumVisibleChildren", &pangolin::View::NumVisibleChildren);
  }
}  // py_pangolin
