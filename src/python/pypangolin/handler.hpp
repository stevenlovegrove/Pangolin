//
// Copyright (c) Andrey Mnatsakanov
//

#ifndef PY_PANGOLIN_HANDLER_HPP
#define PY_PANGOLIN_HANDLER_HPP

#include <pybind11/pybind11.h>
#include <pangolin/handler/handler.h>
#include <pangolin/display/view.h>

namespace py_pangolin {
  
  class PyHandler : public pangolin::Handler {
  public:
    using pangolin::Handler::Handler;

    void Keyboard(pangolin::View& v, unsigned char key, int x, int y, bool pressed) override {
      PYBIND11_OVERLOAD(void, pangolin::Handler, Keyboard, v, key, x, y, pressed);
    }
    void Mouse(pangolin::View& v, pangolin::MouseButton button, int x, int y, bool pressed, int button_state) override {
      PYBIND11_OVERLOAD(void, pangolin::Handler, Mouse, v, button, x, y, pressed, button_state);
    }

    void MouseMotion(pangolin::View& v, int x, int y, int button_state) override {
      PYBIND11_OVERLOAD(void, pangolin::Handler, MouseMotion, v, x, y, button_state);
    }
    void PassiveMouseMotion(pangolin::View& v, int x, int y, int button_state) override {
      PYBIND11_OVERLOAD(void, pangolin::Handler, PassiveMouseMotion, v, x, y, button_state);
    }
      
    void Special(pangolin::View& v, pangolin::InputSpecial inType, float x, float y, float p1, float p2, float p3, float p4, int button_state) override{
      PYBIND11_OVERLOAD(void, pangolin::Handler, Special, v, inType, x, y, p1, p2, p3, p4, button_state);
    }
  };

  void bind_handler(pybind11::module &m);

}  // py_pangolin

#endif //PY_PANGOLIN_HANDLER_HPP
