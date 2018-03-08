/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2018 Andrey Mnatsakanov
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <pangolin/factory/factory_registry.h>
#include <pangolin/platform.h>
#include <pangolin/gl/glinclude.h>
#include <pangolin/display/display.h>
#include <pangolin/display/display_internal.h>
#include <pangolin/display/window.h>

#include <pangolin/display/device/EmscriptenWindow.h>

#include <mutex>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

namespace pangolin
{

extern __thread PangolinGl* context;

std::mutex window_mutex;

  static inline const char *emscripten_event_type_to_string(int eventType) {
    const char *events[] = { "(invalid)", "(none)", "keypress", "keydown", "keyup", "click", "mousedown", "mouseup", "dblclick", "mousemove", "wheel", "resize", 
                             "scroll", "blur", "focus", "focusin", "focusout", "deviceorientation", "devicemotion", "orientationchange", "fullscreenchange", "pointerlockchange", 
                             "visibilitychange", "touchstart", "touchend", "touchmove", "touchcancel", "gamepadconnected", "gamepaddisconnected", "beforeunload", 
                             "batterychargingchange", "batterylevelchange", "webglcontextlost", "webglcontextrestored", "mouseenter", "mouseleave", "mouseover", "mouseout", "(invalid)" };
    ++eventType;
    if (eventType < 0) eventType = 0;
    if (eventType >= sizeof(events)/sizeof(events[0])) eventType = sizeof(events)/sizeof(events[0])-1;
    return events[eventType];
  }
  
  EM_BOOL key_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData){
      printf("%s, key: \"%s\", code: \"%s\", location: %lu,%s%s%s%s repeat: %d, locale: \"%s\", char: \"%s\", charCode: %lu, keyCode: %lu, which: %lu\n",
           emscripten_event_type_to_string(eventType), e->key, e->code, e->location, 
           e->ctrlKey ? " CTRL" : "", e->shiftKey ? " SHIFT" : "", e->altKey ? " ALT" : "", e->metaKey ? " META" : "", 
           e->repeat, e->locale, e->charValue, e->charCode, e->keyCode, e->which);
    return false;
  }
  EM_BOOL mouse_callback(int eventType, const EmscriptenMouseEvent *e, void *userData){
    switch(eventType) {
    case EMSCRIPTEN_EVENT_MOUSEDOWN: {// mousedown
      std::cout << "mouse down" << std::endl;
    }
      break;
    case EMSCRIPTEN_EVENT_MOUSEUP: { //mouseup
      std::cout << "mouse up" << std::endl;
    }
      break;
    case EMSCRIPTEN_EVENT_MOUSEMOVE: { //mousemove
      if(e->buttons){
        std::cout<<"motion"<<std::endl;
        pangolin::process::MouseMotion(e->movementX, e->movementY);
      } else {
        pangolin::process::PassiveMouseMotion(e->movementX, e->movementY);
      }
    }
      break;
    case EMSCRIPTEN_EVENT_MOUSEOVER:{
      std::cout << "mouse enter" << std::endl;
    }
      break;
    case EMSCRIPTEN_EVENT_MOUSEOUT:{
      std::cout << "mouse leave" << std::endl;
    }
      break;
    default:
      break;
    }
    return false;
  }
  EM_BOOL wheel_callback(int eventType, const EmscriptenWheelEvent *e, void *userData){
    return false;
  }
  EM_BOOL uievent_callback(int eventType, const EmscriptenUiEvent *e, void *userData){
    return false;
  }
  EM_BOOL focusevent_callback(int eventType, const EmscriptenFocusEvent *e, void *userData){
    return false;
  }
  EM_BOOL deviceorientation_callback(int eventType, const EmscriptenDeviceOrientationEvent *e, void *userData){
    return false;
  }
  EM_BOOL devicemotion_callback(int eventType, const EmscriptenDeviceMotionEvent *e, void *userData){
    return false;
  }
  EM_BOOL orientationchange_callback(int eventType, const EmscriptenOrientationChangeEvent *e, void *userData){
    return false;
  }
  EM_BOOL fullscreenchange_callback(int eventType, const EmscriptenFullscreenChangeEvent *e, void *userData){
    return false;
  }
  EM_BOOL pointerlockchange_callback(int eventType, const EmscriptenPointerlockChangeEvent *e, void *userData){
    return false;    
  }
  EM_BOOL visibilitychange_callback(int eventType, const EmscriptenVisibilityChangeEvent *e, void *userData){
    return false;
  }
  EM_BOOL touch_callback(int eventType, const EmscriptenTouchEvent *e, void *userData){
    return false;
  }
  EM_BOOL gamepad_callback(int eventType, const EmscriptenGamepadEvent *e, void *userData){
    return false;
  }
  const char *beforeunload_callback(int eventType, const void *reserved, void *userData){
    return "Are You sure?";
  }
  EM_BOOL battery_callback(int eventType, const EmscriptenBatteryEvent *e, void *userData){
    return true;
  }
  EM_BOOL webglcontext_callback(int eventType, const void *reserved, void *userData){
    return false;
  }

#define TEST(error) if(error!=EMSCRIPTEN_RESULT_SUCCESS)std::cerr << "error: " << __FILE__<< ":" << __LINE__ << std::endl

  EmscriptenWindow:: EmscriptenWindow(const std::string& title, int width, int height)
  {
    PangolinGl::windowed_size[0] = width;
    PangolinGl::windowed_size[1] = height;
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
    attr.alpha = attr.depth = attr.stencil = attr.antialias = attr.preserveDrawingBuffer = attr.preferLowPowerToHighPerformance = attr.failIfMajorPerformanceCaveat = 1;
    attr.enableExtensionsByDefault = 1;
    attr.premultipliedAlpha = 0;
    attr.majorVersion = 1;
    attr.minorVersion = 0;
    ctx = emscripten_webgl_create_context(0, &attr);
    if( ctx < 0 ) {
      throw std::runtime_error("Pangolin Emscripten: Failed to create window." );
    }

    EMSCRIPTEN_RESULT ret = emscripten_set_keypress_callback(0, this, 1, key_callback);
    TEST(ret);
    ret = emscripten_set_keydown_callback(0, this, 1, key_callback);
    TEST(ret);
    ret = emscripten_set_keyup_callback(0, this, 1, key_callback);
    TEST(ret);
    ret = emscripten_set_click_callback(0, this, 1, mouse_callback);
    TEST(ret);
    ret = emscripten_set_mousedown_callback(0, this, 1, mouse_callback);
    TEST(ret);
    ret = emscripten_set_mouseup_callback(0, this, 1, mouse_callback);
    TEST(ret);
    ret = emscripten_set_dblclick_callback(0, this, 1, mouse_callback);
    TEST(ret);
    ret = emscripten_set_mousemove_callback(0, this, 1, mouse_callback);
    TEST(ret);
    ret = emscripten_set_mouseenter_callback(0, this, 1, mouse_callback);
    TEST(ret);
    ret = emscripten_set_mouseleave_callback(0, this, 1, mouse_callback);
    TEST(ret);
    ret = emscripten_set_mouseover_callback(0, this, 1, mouse_callback);
    TEST(ret);
    ret = emscripten_set_mouseout_callback(0, this, 1, mouse_callback);
    TEST(ret);
    ret = emscripten_set_wheel_callback(0, this, 1, wheel_callback);
    TEST(ret);
    ret = emscripten_set_resize_callback(0, this, 1, uievent_callback);
    TEST(ret);
    ret = emscripten_set_scroll_callback(0, this, 1, uievent_callback);
    TEST(ret);
    ret = emscripten_set_blur_callback(0, this, 1, focusevent_callback);
    TEST(ret);
    ret = emscripten_set_focus_callback(0, this, 1, focusevent_callback);
    TEST(ret);
    ret = emscripten_set_focusin_callback(0, this, 1, focusevent_callback);
    TEST(ret);
    ret = emscripten_set_focusout_callback(0, this, 1, focusevent_callback);
    TEST(ret);
    ret = emscripten_set_deviceorientation_callback(this, 1, deviceorientation_callback);
    TEST(ret);
    ret = emscripten_set_devicemotion_callback(this, 1, devicemotion_callback);
    TEST(ret);
    //    ret = emscripten_set_orientationchange_callback(0, 1, orientationchange_callback);
    //TEST(ret);
    ret = emscripten_set_fullscreenchange_callback(0, this, 1, fullscreenchange_callback);
    TEST(ret);
    ret = emscripten_set_pointerlockchange_callback(0, this, 1, pointerlockchange_callback);
    TEST(ret);
    ret = emscripten_set_visibilitychange_callback(this, 1, visibilitychange_callback);
    TEST(ret);
    ret = emscripten_set_touchstart_callback(0, this, 1, touch_callback);
    TEST(ret);
    ret = emscripten_set_touchend_callback(0, this, 1, touch_callback);
    TEST(ret);
    ret = emscripten_set_touchmove_callback(0, this, 1, touch_callback);
    TEST(ret);
    ret = emscripten_set_touchcancel_callback(0, this, 1, touch_callback);
    TEST(ret);
    ret = emscripten_set_gamepadconnected_callback(this, 1, gamepad_callback);
    TEST(ret);
    ret = emscripten_set_gamepaddisconnected_callback(this, 1, gamepad_callback);
    TEST(ret);
    ret = emscripten_set_beforeunload_callback(this, beforeunload_callback);
    TEST(ret);
    //ret = emscripten_set_batterychargingchange_callback(0, battery_callback);
    //TEST(ret);
    // ret = emscripten_set_batterylevelchange_callback(0, battery_callback);
    //TEST(ret);
    ret = emscripten_set_webglcontextlost_callback(0, this, 1, webglcontext_callback);
    TEST(ret);
    ret = emscripten_set_webglcontextrestored_callback(0, this, 1, webglcontext_callback);
    TEST(ret);

    EM_ASM(Module['noExitRuntime'] = true);
  }
  
  EmscriptenWindow::~ EmscriptenWindow()
  {
    emscripten_webgl_destroy_context(ctx);
  }

  void EmscriptenWindow::MakeCurrent()
  {
    emscripten_webgl_make_context_current(ctx);
    emscripten_set_canvas_element_size("#canvas", PangolinGl::windowed_size[0], PangolinGl::windowed_size[1]);
    pangolin::glEngine().prog_fixed.Bind();
    context = this;
  }

  void  EmscriptenWindow::ToggleFullscreen()
  {
  }
  
  void EmscriptenWindow::Move(int x, int y)
  {
  }

  void EmscriptenWindow::Resize(unsigned int w, unsigned int h)
  {
  }
  
  void EmscriptenWindow::ProcessEvents(){

  }

  void  EmscriptenWindow::SwapBuffers() {
  }

  std::unique_ptr<WindowInterface> CreateEmscriptenWindowAndBind(const std::string& window_title, const int w, const int h)
  {
    EmscriptenWindow* win = new EmscriptenWindow(window_title, w, h);
    return std::unique_ptr<WindowInterface>(win);
  }

  PANGOLIN_REGISTER_FACTORY(EmscriptenWindow)
  {
    struct EmscriptenWindowFactory : public FactoryInterface<WindowInterface> {
      std::unique_ptr<WindowInterface> Open(const Uri& uri) override {
        const std::string window_title = uri.Get<std::string>("window_title", "window");
        const int w = uri.Get<int>("w", 640);
        const int h = uri.Get<int>("h", 480);
        return std::unique_ptr<WindowInterface>(CreateEmscriptenWindowAndBind(window_title, w, h));
      }
    };

    auto factory = std::make_shared<EmscriptenWindowFactory>();
    FactoryRegistry<WindowInterface>::I().RegisterFactory(factory, 10, "emscriptenwindow");
  }

}
