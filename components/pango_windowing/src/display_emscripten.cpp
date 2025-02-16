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
#include <pangolin/windowing/window.h>
#include <pangolin/windowing/EmscriptenWindow.h>

#include <mutex>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

namespace pangolin
{

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

KeyModifier mod_key(const char* key_string){
    if(strlen(key_string)==1) return KeyModifier(0);
    if(strcmp(key_string, "Shift")==0)  return pangolin::KeyModifierShift;
    if(strcmp(key_string, "Control")==0) return pangolin::KeyModifierCtrl;
    if(strcmp(key_string, "Alt")==0) return pangolin::KeyModifierAlt;
    if(strcmp(key_string, "Meta")==0) return pangolin::KeyModifierCmd;
    return KeyModifier(0);
}

std::mutex window_mutex;

pangolin::KeyModifierBitmask GetKeyModifierBitmask(const EmscriptenKeyboardEvent *event)
{
    pangolin::KeyModifierBitmask mask;
    if(event->shiftKey) mask |=  pangolin::KeyModifierShift;
    if(event->ctrlKey) mask |=  pangolin::KeyModifierCtrl;
    if(event->altKey) mask |=  pangolin::KeyModifierAlt;
    if(event->metaKey) mask |=  pangolin::KeyModifierCmd;
    return mask;
}

pangolin::KeyModifierBitmask GetKeyModifierBitmask(const EmscriptenMouseEvent *event)
{
    pangolin::KeyModifierBitmask mask;
    if(event->shiftKey) mask |=  pangolin::KeyModifierShift;
    if(event->ctrlKey) mask |=  pangolin::KeyModifierCtrl;
    if(event->altKey) mask |=  pangolin::KeyModifierAlt;
    if(event->metaKey) mask |=  pangolin::KeyModifierCmd;
    return mask;
}

EM_BOOL key_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData){
    EmscriptenWindow* w=(EmscriptenWindow*)userData;

    if(eventType==EMSCRIPTEN_EVENT_KEYPRESS)
        return false;

    const KeyModifier mod = mod_key(e->key);
    if(mod == 0) {
        // Not a modifier key
        int key = spec_key(e->key);
        if(key != -1){
            w->KeyboardSignal(KeyboardEvent({
                {(float)w->x, (float)w->y, GetKeyModifierBitmask(e)},
                (uint8_t)key, eventType==EMSCRIPTEN_EVENT_KEYDOWN
            }));
        }
        if(strlen(e->key)==1){
            w->KeyboardSignal(KeyboardEvent({
                {(float)w->x, (float)w->y, GetKeyModifierBitmask(e)},
                (uint8_t)((e->ctrlKey?PANGO_CTRL:0) + e->key[0]),
                eventType==EMSCRIPTEN_EVENT_KEYDOWN
            }));
        }
    }else{
        w->key_modifier_state.set(mod, eventType==EMSCRIPTEN_EVENT_KEYDOWN);
    }
    return false;
}

EM_BOOL mouse_callback(int eventType, const EmscriptenMouseEvent *e, void *userData){
    EmscriptenWindow* w=(EmscriptenWindow*)userData;
    w->x = e->targetX;
    w->y = e->targetY;

    switch(eventType) {
    case EMSCRIPTEN_EVENT_MOUSEDOWN:
    case EMSCRIPTEN_EVENT_MOUSEUP:
        w->MouseSignal(MouseEvent({
            {(float)w->x, (float)w->y, GetKeyModifierBitmask(e)},
            e->button, eventType == EMSCRIPTEN_EVENT_MOUSEDOWN
        }));
        break;
    case EMSCRIPTEN_EVENT_MOUSEMOVE:
        if(e->buttons){
            w->MouseMotionSignal(MouseMotionEvent({(float)w->x, (float)w->y, GetKeyModifierBitmask(e)}));
        } else {
            w->PassiveMouseMotionSignal(MouseMotionEvent({(float)w->x, (float)w->y, GetKeyModifierBitmask(e)}));
        }
        break;
    case EMSCRIPTEN_EVENT_MOUSEOVER:
        break;
    case EMSCRIPTEN_EVENT_MOUSEOUT:
        break;
    default:
        break;
    }
    return false;
}

EM_BOOL wheel_callback(int /*eventType*/, const EmscriptenWheelEvent *e, void *userData){
    EmscriptenWindow* w=(EmscriptenWindow*)userData;
    w->SpecialInputSignal(SpecialInputEvent({
        {(float)w->x, (float)w->y, w->key_modifier_state},
        InputSpecialScroll,
        {(float)e->deltaX, (float)e->deltaY, 0, 0}
    }));
    return true;
}
EM_BOOL uievent_callback(int eventType, const EmscriptenUiEvent */*e*/, void *userData){
    EmscriptenWindow* w=(EmscriptenWindow*)userData;
    switch(eventType) {
    case EMSCRIPTEN_EVENT_RESIZE:
        int width, height;
        emscripten_get_canvas_element_size(em_dom_id, &width, &height);
        w->ResizeSignal(WindowResizeEvent({width, height}));
        break;
    }
    return true;
}

#define TEST(error) if(error!=EMSCRIPTEN_RESULT_SUCCESS)std::cerr << "error: " << __FILE__<< ":" << __LINE__ << std::endl

EmscriptenWindow:: EmscriptenWindow()
    : done_init_events(false)
{
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
    attr.majorVersion = 2;
    attr.minorVersion = 0;
    attr.stencil = true;
    attr.depth = true;
    ctx = emscripten_webgl_create_context(em_dom_id, &attr);
    if( ctx < 0 ) {
        throw std::runtime_error("Pangolin Emscripten: Failed to create window." );
    }
    // Try to enable some extensions we'll probably need.
    emscripten_webgl_enable_extension(ctx, "EXT_float_blend");

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

    EM_ASM(Module['noExitRuntime'] = true);
}

#undef TEST

EmscriptenWindow::~ EmscriptenWindow()
{
    emscripten_webgl_destroy_context(ctx);
}

void EmscriptenWindow::MakeCurrent()
{
    emscripten_webgl_make_context_current(ctx);
    pangolin::glEngine().prog_fixed.Bind();
}

void EmscriptenWindow::RemoveCurrent()
{
    pangolin::glEngine().prog_fixed.Unbind();
}

void EmscriptenWindow::ShowFullscreen(const TrueFalseToggle)
{
    // Not implemented
}

void EmscriptenWindow::Move(int /*x*/, int /*y*/)
{
    // Not implemented
}

void EmscriptenWindow::Resize(unsigned int w, unsigned int h)
{
    emscripten_set_canvas_element_size(em_dom_id, w, h);
}

void EmscriptenWindow::ProcessEvents()
{
    // Emscripten will trigger callbacks.
    if(!done_init_events) {
        done_init_events = true;
        int width, height;
        emscripten_get_canvas_element_size(em_dom_id, &width, &height);
        ResizeSignal(WindowResizeEvent({width, height}));
    }
}

void  EmscriptenWindow::SwapBuffers() {
    emscripten_sleep(10);
}

PANGOLIN_REGISTER_FACTORY(EmscriptenWindow)
{
    struct EmscriptenWindowFactory : public TypedFactoryInterface<WindowInterface> {
        std::map<std::string,Precedence> Schemes() const override
        {
            return {{"emscripten",10},{"default",100}};
        }
        const char* Description() const override
        {
            return "Use WebGL window";
        }
        ParamSet Params() const override
        {
            return {{
                {"window_title","-","Ignored"},
                {"w","640","Ignored"},
                {"h","480","Ignored"},
                {PARAM_GL_PROFILE,{},"Ignored for now"},
            }};
        }

        std::unique_ptr<WindowInterface> Open(const Uri& /*uri*/) override {
            // We're going to be naughty and actually ignore the title, width and height,
            // but list them as parameters to be compatible with other windowing libs.
            return std::unique_ptr<WindowInterface>(new EmscriptenWindow());
        }
    };
    return FactoryRegistry::I()->RegisterFactory<WindowInterface>(std::make_shared<EmscriptenWindowFactory>());
}

}

#include <emscripten/bind.h>

std::string pango_get_exception_message(intptr_t exceptionPtr) {
    return std::string(reinterpret_cast<std::exception *>(exceptionPtr)->what());
}

EMSCRIPTEN_BINDINGS(Bindings) {
    emscripten::function("pango_get_exception_message", &pango_get_exception_message);
};
