/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2016 Steven Lovegrove
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

#pragma once
#include <exception>
#include <functional>
#include <array>
#include <string>
#include <memory>
#include <pangolin/platform.h>
#include <pangolin/utils/signal_slot.h>
#include <pangolin/utils/true_false_toggle.h>
#include <pangolin/utils/uri.h>
#include <pangolin/windowing/handler_bitsets.h>

namespace pangolin
{

// CreateWindowAndBind parameter key names.
constexpr char PARAM_DISPLAYNAME[]    = "DISPLAYNAME\0";    // std::string
constexpr char PARAM_DOUBLEBUFFER[]   = "DOUBLEBUFFER\0";   // bool
constexpr char PARAM_SAMPLE_BUFFERS[] = "SAMPLE_BUFFERS\0"; // int
constexpr char PARAM_SAMPLES[]        = "SAMPLES\0";        // int
constexpr char PARAM_HIGHRES[]        = "HIGHRES\0";        // bool

struct PANGOLIN_EXPORT WindowResizeEvent
{
    int width;
    int height;
};

struct PANGOLIN_EXPORT WindowInputEvent
{
    float x;
    float y;
    KeyModifierBitmask key_modifiers;
};

struct PANGOLIN_EXPORT KeyboardEvent : public WindowInputEvent
{
    unsigned char key;
    bool pressed;
};

struct PANGOLIN_EXPORT MouseEvent : public WindowInputEvent
{
    int button;
    bool pressed;
};

struct PANGOLIN_EXPORT MouseMotionEvent : public WindowInputEvent
{
};

struct PANGOLIN_EXPORT SpecialInputEvent : public WindowInputEvent
{
    InputSpecial inType;
    float p[4];
};

class PANGOLIN_EXPORT GlContextInterface
{
public:
    virtual ~GlContextInterface() {}
};

class PANGOLIN_EXPORT WindowInterface
{
public:
    virtual ~WindowInterface() {}

    /// Move window to (\param x, \param y) on screen
    /// measured in pixels.
    virtual void Move(int x, int y) = 0;

    /// Resize window to (\param x, \param y) on screen
    /// measured in pixels
    virtual void Resize(unsigned int w, unsigned int h) = 0;

    /// If supported by this windowing system, turn on,
    /// turn off, or toggle fullscreen mode.
    virtual void ShowFullscreen(const TrueFalseToggle on_off) = 0;

    /// Make this the active context for graphic rendering.
    virtual void MakeCurrent() = 0;

    /// Unregister this context from being the active rendering context.
    virtual void RemoveCurrent() = 0;

    /// Process all windowing events to keep window alive
    /// and emit interaction callbacks (e.g. mouse, keyboard, resize)
    virtual void ProcessEvents() = 0;

    /// If double-buffered rendering is enabled, swap the
    /// front and back buffers revealing the recent renders
    /// to the back buffer.
    virtual void SwapBuffers() = 0;

    sigslot::signal<>                    CloseSignal;
    sigslot::signal<WindowResizeEvent>   ResizeSignal;
    sigslot::signal<KeyboardEvent> KeyboardSignal;
    sigslot::signal<MouseEvent>    MouseSignal;
    sigslot::signal<MouseMotionEvent> MouseMotionSignal;
    sigslot::signal<MouseMotionEvent> PassiveMouseMotionSignal;
    sigslot::signal<SpecialInputEvent> SpecialInputSignal;
};

//! Open Window Interface from Uri specification
PANGOLIN_EXPORT
std::unique_ptr<WindowInterface> ConstructWindow(const Uri& uri);

}
