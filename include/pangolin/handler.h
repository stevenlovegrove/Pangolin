/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2013 Steven Lovegrove
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

#ifndef PANGOLIN_HANDLER_H
#define PANGOLIN_HANDLER_H

#include "opengl_render_state.h"

namespace pangolin
{

// Forward declarations
struct View;

// Supported Key modifiers for GlobalKeyPressCallback.
// e.g. PANGO_CTRL + 'r', PANGO_SPECIAL + GLUT_KEY_RIGHT, etc.
const int PANGO_SPECIAL = 128;
const int PANGO_CTRL = -96;
const int PANGO_OPTN = 132;

// Special Keys (same as GLUT_ defines)
const int PANGO_KEY_F1        = 1;
const int PANGO_KEY_F2        = 2;
const int PANGO_KEY_F3        = 3;
const int PANGO_KEY_F4        = 4;
const int PANGO_KEY_F5        = 5;
const int PANGO_KEY_F6        = 6;
const int PANGO_KEY_F7        = 7;
const int PANGO_KEY_F8        = 8;
const int PANGO_KEY_F9        = 9;
const int PANGO_KEY_F10       = 10;
const int PANGO_KEY_F11       = 11;
const int PANGO_KEY_F12       = 12;
const int PANGO_KEY_LEFT      = 100;
const int PANGO_KEY_UP        = 101;
const int PANGO_KEY_RIGHT     = 102;
const int PANGO_KEY_DOWN      = 103;
const int PANGO_KEY_PAGE_UP   = 104;
const int PANGO_KEY_PAGE_DOWN = 105;
const int PANGO_KEY_HOME      = 106;
const int PANGO_KEY_END	      = 107;
const int PANGO_KEY_INSERT	  = 108;

enum MouseButton
{
    MouseButtonLeft = 1,
    MouseButtonMiddle = 2,
    MouseButtonRight = 4,
    MouseWheelUp = 8,
    MouseWheelDown = 16
};

enum KeyModifier
{
    KeyModifierShift = 1<<16,
    KeyModifierCtrl  = 1<<17,
    KeyModifierAlt   = 1<<18,
    KeyModifierCmd   = 1<<19
};

enum InputSpecial
{
    InputSpecialScroll,
    InputSpecialZoom,
    InputSpecialRotate,
    InputSpecialTablet
};

//! @brief Input Handler base class with virtual methods which recurse
//! into sub-displays
struct Handler
{
    virtual ~Handler() {}
    virtual void Keyboard(View&, unsigned char key, int x, int y, bool pressed);
    virtual void Mouse(View&, MouseButton button, int x, int y, bool pressed, int button_state);
    virtual void MouseMotion(View&, int x, int y, int button_state);
    virtual void PassiveMouseMotion(View&, int x, int y, int button_state);
    virtual void Special(View&, InputSpecial inType, float x, float y, float p1, float p2, float p3, float p4, int button_state);
};

struct HandlerScroll : Handler
{
    void Mouse(View&, MouseButton button, int x, int y, bool pressed, int button_state);
    void Special(View&, InputSpecial inType, float x, float y, float p1, float p2, float p3, float p4, int button_state);
};

struct Handler3D : Handler
{

    Handler3D(OpenGlRenderState& cam_state, AxisDirection enforce_up=AxisNone, float trans_scale=0.01f, float zoom_fraction=1.0f/50.0f);

    virtual void GetPosNormal(View& view, int x, int y, GLdouble p[3], GLdouble Pw[3], GLdouble Pc[3], GLdouble n[3], GLdouble default_z = 1.0);
    void Keyboard(View&, unsigned char key, int x, int y, bool pressed);
    void Mouse(View&, MouseButton button, int x, int y, bool pressed, int button_state);
    void MouseMotion(View&, int x, int y, int button_state);
    void Special(View&, InputSpecial inType, float x, float y, float p1, float p2, float p3, float p4, int button_state);

protected:
    OpenGlRenderState* cam_state;
    const static int hwin = 8;
    AxisDirection enforce_up;
    float tf; // translation factor
    float zf; // zoom fraction
    CameraSpec cameraspec;
    GLfloat last_z;
    GLint last_pos[2];
    GLdouble rot_center[3];

    GLdouble p[3];
    GLdouble Pw[3];
    GLdouble Pc[3];
    GLdouble n[3];
};

struct PassThroughHandler : Handler
{
  PassThroughHandler() {}

  void SetPassThroughView(View& v) {
    pass_through_view = &v;
  }

  virtual void Keyboard(View&, unsigned char key, int x, int y, bool pressed);
  virtual void Mouse(View&, MouseButton button, int x, int y, bool pressed, int button_state);
  virtual void MouseMotion(View&, int x, int y, int button_state);
  virtual void PassiveMouseMotion(View&, int x, int y, int button_state);
  virtual void Special(View&, InputSpecial inType, float x, float y, float p1, float p2, float p3, float p4, int button_state);

  View* pass_through_view;
};

static Handler StaticHandler;
static HandlerScroll StaticHandlerScroll;

}

#endif // PANGOLIN_HANDLER_H
