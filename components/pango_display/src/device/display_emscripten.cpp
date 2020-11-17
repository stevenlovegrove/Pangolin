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

constexpr char em_dom_id[] = "#canvas";

int spec_key(const char* key_string){
    if(strlen(key_string)==1){
        return -1;
    }
    if(strcmp(key_string, "F1")==0) return PANGO_SPECIAL + PANGO_KEY_F1;
    if(strcmp(key_string, "F2")==0) return PANGO_SPECIAL + PANGO_KEY_F2;
    if(strcmp(key_string, "F3")==0) return PANGO_SPECIAL + PANGO_KEY_F3;
    if(strcmp(key_string, "F4")==0) return PANGO_SPECIAL + PANGO_KEY_F4;
    if(strcmp(key_string, "F5")==0) return PANGO_SPECIAL + PANGO_KEY_F5;
    if(strcmp(key_string, "F6")==0) return PANGO_SPECIAL + PANGO_KEY_F6;
    if(strcmp(key_string, "F7")==0) return PANGO_SPECIAL + PANGO_KEY_F7;
    if(strcmp(key_string, "F8")==0) return PANGO_SPECIAL + PANGO_KEY_F8;
    if(strcmp(key_string, "F9")==0) return PANGO_SPECIAL + PANGO_KEY_F9;
    if(strcmp(key_string, "F10")==0)return PANGO_SPECIAL + PANGO_KEY_F10;
    if(strcmp(key_string, "F11")==0)return PANGO_SPECIAL + PANGO_KEY_F11;
    if(strcmp(key_string, "F12")==0)return PANGO_SPECIAL + PANGO_KEY_F12;
    if(strcmp(key_string, "ArrowLeft")==0)return PANGO_SPECIAL + PANGO_KEY_LEFT;
    if(strcmp(key_string, "ArrowUp")==0)return PANGO_SPECIAL + PANGO_KEY_UP;
    if(strcmp(key_string, "ArrowRight")==0)return PANGO_SPECIAL + PANGO_KEY_RIGHT;
    if(strcmp(key_string, "ArrowDown")==0)return PANGO_SPECIAL + PANGO_KEY_DOWN;
    if(strcmp(key_string, "PageUp")==0)return PANGO_SPECIAL + PANGO_KEY_PAGE_UP;
    if(strcmp(key_string, "PageDown")==0)return PANGO_SPECIAL + PANGO_KEY_PAGE_DOWN;
    if(strcmp(key_string, "Home")==0)return PANGO_SPECIAL + PANGO_KEY_HOME;
    if(strcmp(key_string, "End")==0)return PANGO_SPECIAL + PANGO_KEY_END;
    if(strcmp(key_string, "Insert")==0)return PANGO_SPECIAL + PANGO_KEY_INSERT;
    return -1;
}

int mod_key(const char* key_string, bool pressed){
    if(strlen(key_string)==1){
        return -1;
    }
    if(strcmp(key_string, "Shift")==0){
        if(pressed) {
            pangolin::context->mouse_state |=  pangolin::KeyModifierShift;
        }else{
            pangolin::context->mouse_state &= ~pangolin::KeyModifierShift;
        }
        return 0;
    }
    if(strcmp(key_string, "Control")==0){
        if(pressed) {
            pangolin::context->mouse_state |=  pangolin::KeyModifierCtrl;
        }else{
            pangolin::context->mouse_state &= ~pangolin::KeyModifierCtrl;
        }
        return 0;
    }
    if(strcmp(key_string, "Alt")==0){
        if(pressed) {
            pangolin::context->mouse_state |=  pangolin::KeyModifierAlt;
        }else{
            pangolin::context->mouse_state &= ~pangolin::KeyModifierAlt;
        }
        return 0;
    }
    if(strcmp(key_string, "Meta")==0){
        if(pressed) {
            pangolin::context->mouse_state |=  pangolin::KeyModifierCmd;
        }else{
            pangolin::context->mouse_state &= ~pangolin::KeyModifierCmd;
        }
        return 0;
    }
    return -1;
}

std::mutex window_mutex;

EM_BOOL key_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData){
    std::cout << "key_callback" << std::endl;

    EmscriptenWindow* w=(EmscriptenWindow*)userData;
    if(eventType==EMSCRIPTEN_EVENT_KEYPRESS)return false;
    int key = mod_key(e->key, eventType==EMSCRIPTEN_EVENT_KEYDOWN?true:false);
    if(key != -1){
        return false;
    }
    key = spec_key(e->key);
    if(key != -1){
        if(eventType==EMSCRIPTEN_EVENT_KEYDOWN) {
            pangolin::process::Keyboard(key, w->x, w->y);
        }else{
            pangolin::process::KeyboardUp(key, w->x, w->y);
        }
        return false;
    }
    if(strlen(e->key)==1){
        if(eventType==EMSCRIPTEN_EVENT_KEYDOWN) {
            pangolin::process::Keyboard((e->ctrlKey?PANGO_CTRL:0) + e->key[0], w->x, w->y);
        }else{
            pangolin::process::KeyboardUp((e->ctrlKey?PANGO_CTRL:0) + e->key[0], w->x, w->y);
        }
    }
    return false;
}

EM_BOOL mouse_callback(int eventType, const EmscriptenMouseEvent *e, void *userData){
    int state = 1;
    EmscriptenWindow* w=(EmscriptenWindow*)userData;
    w->x = e->targetX;
    w->y = e->targetY;

    switch(eventType) {
    case EMSCRIPTEN_EVENT_MOUSEDOWN:
        state=0;
    case EMSCRIPTEN_EVENT_MOUSEUP: {
        pangolin::process::Mouse(e->button, state, w->x, w->y);
        break;
    }
        break;
    case EMSCRIPTEN_EVENT_MOUSEMOVE: {
        if(e->buttons){
            pangolin::process::MouseMotion(w->x, w->y);
        } else {
            pangolin::process::PassiveMouseMotion(w->x, w->y);
        }
    }
        break;
    case EMSCRIPTEN_EVENT_MOUSEOVER:{
    }
        break;
    case EMSCRIPTEN_EVENT_MOUSEOUT:{
    }
        break;
    default:
        break;
    }
    return false;
}

EM_BOOL wheel_callback(int eventType, const EmscriptenWheelEvent *e, void *userData){
    EmscriptenWindow* w=(EmscriptenWindow*)userData;
    pangolin::process::SpecialInput(InputSpecialScroll, w->x, w->windowed_size[1] - w->y, e->deltaX, -e->deltaY, 0, 0);
    return true;
}
EM_BOOL uievent_callback(int eventType, const EmscriptenUiEvent *e, void *userData){
    std::cout << "uievent_callback" << std::endl;
    switch(eventType) {
    case EMSCRIPTEN_EVENT_RESIZE:
        int width, height;
        emscripten_get_canvas_element_size(em_dom_id, &width, &height);
        pangolin::process::Resize(width, height);
        break;
    }
    return true;
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
    return nullptr; // "Are You sure?";
}
EM_BOOL battery_callback(int eventType, const EmscriptenBatteryEvent *e, void *userData){
    return true;
}
EM_BOOL webglcontext_callback(int eventType, const void *reserved, void *userData){
    return false;
}

#define TEST(error) if(error!=EMSCRIPTEN_RESULT_SUCCESS)std::cerr << "error: " << __FILE__<< ":" << __LINE__ << std::endl

EmscriptenWindow:: EmscriptenWindow(const std::string& /*title*/, int width, int height)
{
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
    attr.majorVersion = 2;
    attr.minorVersion = 0;
    ctx = emscripten_webgl_create_context(em_dom_id, &attr);
    if( ctx < 0 ) {
        throw std::runtime_error("Pangolin Emscripten: Failed to create window." );
    }

    emscripten_get_canvas_element_size(em_dom_id, &width, &height);

    PangolinGl::windowed_size[0] = width;
    PangolinGl::windowed_size[1] = height;

    EMSCRIPTEN_RESULT ret;
    ret = emscripten_set_keypress_callback(em_dom_id, this, 1, key_callback);
    TEST(ret);
    ret = emscripten_set_keydown_callback(em_dom_id, this, 1, key_callback);
    TEST(ret);
    ret = emscripten_set_keyup_callback(em_dom_id, this, 1, key_callback);
    TEST(ret);
    ret = emscripten_set_click_callback(em_dom_id, this, 1, mouse_callback);
    TEST(ret);
    ret = emscripten_set_mousedown_callback(em_dom_id, this, 1, mouse_callback);
    TEST(ret);
    ret = emscripten_set_mouseup_callback(em_dom_id, this, 1, mouse_callback);
    TEST(ret);
    ret = emscripten_set_dblclick_callback(em_dom_id, this, 1, mouse_callback);
    TEST(ret);
    ret = emscripten_set_mousemove_callback(em_dom_id, this, 1, mouse_callback);
    TEST(ret);
    ret = emscripten_set_mouseenter_callback(em_dom_id, this, 1, mouse_callback);
    TEST(ret);
    ret = emscripten_set_mouseleave_callback(em_dom_id, this, 1, mouse_callback);
    TEST(ret);
    ret = emscripten_set_mouseover_callback(em_dom_id, this, 1, mouse_callback);
    TEST(ret);
    ret = emscripten_set_mouseout_callback(em_dom_id, this, 1, mouse_callback);
    TEST(ret);
    ret = emscripten_set_wheel_callback(em_dom_id, this, 1, wheel_callback);
    TEST(ret);
    ret = emscripten_set_resize_callback(em_dom_id, this, 1, uievent_callback);
    TEST(ret);
    //    ret = emscripten_set_scroll_callback(em_dom_id, this, 1, uievent_callback);
    //    TEST(ret);
    ret = emscripten_set_blur_callback(em_dom_id, this, 1, focusevent_callback);
    TEST(ret);
    ret = emscripten_set_focus_callback(em_dom_id, this, 1, focusevent_callback);
    TEST(ret);
    ret = emscripten_set_focusin_callback(em_dom_id, this, 1, focusevent_callback);
    TEST(ret);
    ret = emscripten_set_focusout_callback(em_dom_id, this, 1, focusevent_callback);
    TEST(ret);
    ret = emscripten_set_deviceorientation_callback(this, 1, deviceorientation_callback);
    TEST(ret);
    ret = emscripten_set_devicemotion_callback(this, 1, devicemotion_callback);
    TEST(ret);
    //    ret = emscripten_set_orientationchange_callback(em_dom_id, 1, orientationchange_callback);
    //TEST(ret);
    ret = emscripten_set_fullscreenchange_callback(em_dom_id, this, 1, fullscreenchange_callback);
    TEST(ret);
    ret = emscripten_set_pointerlockchange_callback(em_dom_id, this, 1, pointerlockchange_callback);
    TEST(ret);
    ret = emscripten_set_visibilitychange_callback(this, 1, visibilitychange_callback);
    TEST(ret);
    ret = emscripten_set_touchstart_callback(em_dom_id, this, 1, touch_callback);
    TEST(ret);
    ret = emscripten_set_touchend_callback(em_dom_id, this, 1, touch_callback);
    TEST(ret);
    ret = emscripten_set_touchmove_callback(em_dom_id, this, 1, touch_callback);
    TEST(ret);
    ret = emscripten_set_touchcancel_callback(em_dom_id, this, 1, touch_callback);
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
    ret = emscripten_set_webglcontextlost_callback(em_dom_id, this, 1, webglcontext_callback);
    TEST(ret);
    ret = emscripten_set_webglcontextrestored_callback(em_dom_id, this, 1, webglcontext_callback);
    TEST(ret);

    EM_ASM(Module['noExitRuntime'] = true);
}

EmscriptenWindow::~ EmscriptenWindow()
{
    emscripten_webgl_destroy_context(ctx);
}

void EmscriptenWindow::MakeCurrent()
{
    context = this;
    emscripten_webgl_make_context_current(ctx);
    pangolin::glEngine().prog_fixed.Bind();
}

void  EmscriptenWindow::ToggleFullscreen()
{
}

void EmscriptenWindow::Move(int x, int y)
{
}

void EmscriptenWindow::Resize(unsigned int w, unsigned int h)
{
    emscripten_set_canvas_element_size(em_dom_id, w, h);
}

void EmscriptenWindow::ProcessEvents(){

}

void  EmscriptenWindow::SwapBuffers() {
    emscripten_sleep(10);
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
    FactoryRegistry<WindowInterface>::I().RegisterFactory(factory, 10, "emscripten");
    FactoryRegistry<WindowInterface>::I().RegisterFactory(factory, 100, "default");
}

}

#include <emscripten/bind.h>

std::string pango_get_exception_message(intptr_t exceptionPtr) {
    return std::string(reinterpret_cast<std::exception *>(exceptionPtr)->what());
}

EMSCRIPTEN_BINDINGS(Bindings) {
    emscripten::function("pango_get_exception_message", &pango_get_exception_message);
};
