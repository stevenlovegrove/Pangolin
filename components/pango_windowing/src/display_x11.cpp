/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011-2018 Steven Lovegrove, Andrey Mnatsakanov
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

// Code based on public domain sample at
// https://github.com/chadversary/gl-examples/blob/master/src/x11-egl/create-x-egl-gl-surface.cpp

#include <pangolin/factory/factory_registry.h>
#include <pangolin/platform.h>
#include <pangolin/gl/glinclude.h>
#include <pangolin/windowing/X11Window.h>

#include <mutex>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <cstdarg>
#include <unordered_map>

#include <EGL/egl.h>

static const std::unordered_map<EGLint, std::string> egl_error_msg = {
    {EGL_SUCCESS, "EGL_SUCCESS"},
    {EGL_NOT_INITIALIZED, "EGL_NOT_INITIALIZED"},
    {EGL_BAD_ACCESS, "EGL_BAD_ACCESS"},
    {EGL_BAD_ALLOC, "EGL_BAD_ALLOC"},
    {EGL_BAD_ATTRIBUTE, "EGL_BAD_ATTRIBUTE"},
    {EGL_BAD_CONTEXT, "EGL_BAD_CONTEXT"},
    {EGL_BAD_CONFIG, "EGL_BAD_CONFIG"},
    {EGL_BAD_CURRENT_SURFACE, "EGL_BAD_CURRENT_SURFACE"},
    {EGL_BAD_DISPLAY, "EGL_BAD_DISPLAY"},
    {EGL_BAD_SURFACE, "EGL_BAD_SURFACE"},
    {EGL_BAD_MATCH, "EGL_BAD_MATCH"},
    {EGL_BAD_PARAMETER, "EGL_BAD_PARAMETER"},
    {EGL_BAD_NATIVE_PIXMAP, "EGL_BAD_NATIVE_PIXMAP"},
    {EGL_BAD_NATIVE_WINDOW, "EGL_BAD_NATIVE_WINDOW"},
    {EGL_CONTEXT_LOST, "EGL_CONTEXT_LOST"},
};

#define CheckEGLDieOnError() pangolin::_CheckEGLDieOnError( __FILE__, __LINE__ );
namespace pangolin {
inline void _CheckEGLDieOnError( const char *sFile, const int nLine )
{
    const EGLint eglError = eglGetError();
    if( eglError != EGL_SUCCESS ) {
        pango_print_error("EGL Error: %s (%x)\n", egl_error_msg.at(eglError).c_str(), eglError);
        pango_print_error("In: %s, line %d\n", sFile, nLine);
        exit(EXIT_FAILURE);
    }
}
}

namespace pangolin {
std::string getEGLErrorString() {
    const EGLint eglError = eglGetError();
    std::ostringstream error_stream;
    error_stream << egl_error_msg.at(eglError) << " (" << std::hex << eglError << ")";
    return error_stream.str();
}
}

namespace pangolin
{

void
error_fatal(const char* format, ...) {
    printf("error: ");

    va_list va;
    va_start(va, format);
    vprintf(format, va);
    va_end(va);

    printf("\n");
    exit(1);
}

std::mutex window_mutex;
std::weak_ptr<X11GlContext> global_gl_context;

const long EVENT_MASKS = ButtonPressMask|ButtonReleaseMask|StructureNotifyMask|ButtonMotionMask|PointerMotionMask|KeyPressMask|KeyReleaseMask|FocusChangeMask;

// Adapted from: http://www.opengl.org/resources/features/OGLextensions/
bool isExtensionSupported(const char *extList, const char *extension)
{
    /* Extension names should not have spaces. */
    const char* where = strchr(extension, ' ');
    if (where || *extension == '\0') {
        return false;
    }

    for(const char* start=extList;;) {
        where = strstr(start, extension);
        if (!where) {
            break;
        }

        const char *terminator = where + strlen(extension);

        if ( where == start || *(where - 1) == ' ' ) {
            if ( *terminator == ' ' || *terminator == '\0' ) {
                return true;
            }
        }

        start = terminator;
    }

    return false;
}

static bool ctxErrorOccurred = false;
static int ctxErrorHandler( ::Display * /*dpy*/, ::XErrorEvent * ev )
{
    const int buffer_size = 10240;
    char buffer[buffer_size];
    XGetErrorText(ev->display, ev->error_code, buffer, buffer_size );
    pango_print_error("X11 Error: %s\n", buffer);
    ctxErrorOccurred = true;
    return 0;
}

X11GlContext::X11GlContext(std::shared_ptr<X11Display>& xdisplay) : display(xdisplay)
{

    // Install an X error handler so the application won't exit if GL 3.0
    // context allocation fails. Handler is global and shared across all threads.
    ctxErrorOccurred = false;
    XSetErrorHandler(&ctxErrorHandler);

    EGLint ignore;
    EGLBoolean ok;

    ok = eglBindAPI(EGL_OPENGL_API);
    if (!ok)
        error_fatal("eglBindAPI(0x%x) failed: %s", EGL_OPENGL_API, getEGLErrorString().c_str());
    CheckEGLDieOnError();

    egl_display = eglGetDisplay(xdisplay->display);
    if (egl_display == EGL_NO_DISPLAY)
        error_fatal("eglGetDisplay() failed: %s", getEGLErrorString().c_str());
    CheckEGLDieOnError();

    ok = eglInitialize(egl_display, &ignore, &ignore);
    if (!ok)
        error_fatal("eglInitialize() failed: %s", getEGLErrorString().c_str());
    CheckEGLDieOnError();

//    std::cout << "EGL version: " << eglQueryString(egl_display, EGL_VERSION) << std::endl;
//    std::cout << "EGL vendor: " << eglQueryString(egl_display, EGL_VENDOR) << std::endl;
//    std::cout << "EGL apis: " << eglQueryString(egl_display, EGL_CLIENT_APIS) << std::endl;
//    std::cout << "EGL extensions: " << eglQueryString(egl_display, EGL_EXTENSIONS) << std::endl;
//    CheckEGLDieOnError();

    const EGLint egl_config_attribs[] = {
        EGL_COLOR_BUFFER_TYPE,     EGL_RGB_BUFFER,
        EGL_RED_SIZE,              8,
        EGL_GREEN_SIZE,            8,
        EGL_BLUE_SIZE,             8,

        EGL_DEPTH_SIZE,            24,
        EGL_STENCIL_SIZE,          8,

        EGL_SAMPLE_BUFFERS,        0,
        EGL_SAMPLES,               0,

        EGL_SURFACE_TYPE,          EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE,       EGL_OPENGL_BIT,

        EGL_NONE,
    };

    EGLint configs_size = 256;
    EGLConfig* configs = new EGLConfig[configs_size];
    EGLint num_configs;
    ok = eglChooseConfig(
        egl_display,
        egl_config_attribs,
        configs,
        configs_size, // num requested configs
        &num_configs); // num returned configs
    if (!ok)
        error_fatal("eglChooseConfig() failed: %s", getEGLErrorString().c_str());
    if (num_configs == 0)
        error_fatal("failed to find suitable EGLConfig: %s", getEGLErrorString().c_str());
    egl_config = configs[0];
    delete [] configs;
    CheckEGLDieOnError();

    const EGLint egl_context_attribs[] = {
        EGL_NONE,
    };

    egl_context = eglCreateContext(
            egl_display,
            egl_config,
            EGL_NO_CONTEXT,
            egl_context_attribs);
    if (!egl_context)
        error_fatal("eglCreateContext() failed: %s", getEGLErrorString().c_str());
    CheckEGLDieOnError();

    // Check if surface is double buffered.
    EGLint render_buffer;
    ok = eglQueryContext(
        egl_display,
        egl_context,
        EGL_RENDER_BUFFER,
        &render_buffer);
    if (!ok)
        error_fatal("eglQueyContext(EGL_RENDER_BUFFER) failed: %s", getEGLErrorString().c_str());
    if (render_buffer == EGL_SINGLE_BUFFER)
        printf("warn: EGL surface is single buffered\n");
    CheckEGLDieOnError();
}

X11GlContext::~X11GlContext()
{
    // cleanup EGL
    eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if(egl_context) eglDestroyContext(egl_display, egl_context);
    if(egl_surface) eglDestroySurface(egl_display, egl_surface);
    if(egl_display) eglTerminate(egl_display);
}

X11Window::X11Window(
    const std::string& title, int width, int height,
    std::shared_ptr<X11Display>& display, std::shared_ptr<X11GlContext> newglcontext
) : display(display), glcontext(newglcontext), win(0), cmap(0)
{
    // Get a visual
    EGLint vid;
    if(eglGetConfigAttrib(glcontext->egl_display, glcontext->egl_config,
                          EGL_NATIVE_VISUAL_ID, &vid)!=EGL_TRUE)
    {
        error_fatal("eglGetConfigAttrib() failed: %s", getEGLErrorString().c_str());
    }
    CheckEGLDieOnError();

    XVisualInfo x11_visual_info_template;
    x11_visual_info_template.visualid = VisualID(vid);

    int num_visuals;
    XVisualInfo *vi = XGetVisualInfo(
        display->display, VisualIDMask, &x11_visual_info_template, &num_visuals);

    if(!vi)
        error_fatal("XGetVisualInfo() failed");

    // Create colourmap
    XSetWindowAttributes swa;
    swa.background_pixmap = None;
    swa.border_pixel    = 0;
    swa.event_mask      = StructureNotifyMask;
    swa.colormap = cmap = XCreateColormap( display->display,
                                           RootWindow( display->display, vi->screen ),
                                           vi->visual, AllocNone );

    // Create window
    win = XCreateWindow( display->display, RootWindow( display->display, vi->screen ),
                         0, 0, width, height, 0, vi->depth, InputOutput,
                         vi->visual,
                         CWBorderPixel|CWColormap|CWEventMask, &swa );

    if ( !win ) {
        throw std::runtime_error("Pangolin X11: Failed to create window." );
    }

    XFree(vi);

    // set name in window switching (alt-tab) list
    {
      XClassHint class_hint;
      class_hint.res_class = const_cast<char*>(title.c_str());
      class_hint.res_name = const_cast<char*>("");
      XSetClassHint( display->display, win, &class_hint );
    }

    // set window title
    XStoreName( display->display, win, title.c_str() );
    XMapWindow( display->display, win );

    // Request to be notified of these events
    XSelectInput(display->display, win, EVENT_MASKS );

    delete_message = XInternAtom(display->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display->display, win, &delete_message, 1);

    const EGLint egl_surface_attribs[] = {
        EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
        EGL_NONE,
    };

    glcontext->egl_surface = eglCreateWindowSurface(
            glcontext->egl_display,
            glcontext->egl_config,
            win,
            egl_surface_attribs);
    if (!glcontext->egl_surface)
        error_fatal("eglCreateWindowSurface() failed: %s", getEGLErrorString().c_str());
    CheckEGLDieOnError();
}

X11Window::~X11Window()
{
    XDestroyWindow( display->display, win );
    XFreeColormap( display->display, cmap );
}

void X11Window::MakeCurrent(EGLContext ctx)
{
    if(eglMakeCurrent( glcontext->egl_display, glcontext->egl_surface, glcontext->egl_surface, ctx )==EGL_FALSE) {
        const EGLint eglError = eglGetError();
        if(eglError==EGL_BAD_ACCESS) {
            std::cerr << "Received 'EGL_BAD_ACCESS' trying to set current EGL context." << std::endl;
            std::cerr << "When calling 'MakeCurrent()' from a different thread, you need to unset the previous context first by calling 'RemoveCurrent()'." << std::endl;
        }
        else {
            pango_print_error("X11Window::MakeCurrent(): EGL Error %s (%x)\n", egl_error_msg.at(eglError).c_str(), eglError);
        }
        exit(EXIT_FAILURE);
    }
}

void X11Window::MakeCurrent()
{
    CheckEGLDieOnError();
    MakeCurrent(glcontext ? glcontext->egl_context : global_gl_context.lock()->egl_context);
    CheckEGLDieOnError();
}

void X11Window::RemoveCurrent()
{
    eglMakeCurrent(glcontext->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

void X11Window::ShowFullscreen(const TrueFalseToggle true_false)
{
    const Atom _NET_WM_STATE_FULLSCREEN = XInternAtom(display->display, "_NET_WM_STATE_FULLSCREEN", True);
    const Atom _NET_WM_STATE = XInternAtom(display->display, "_NET_WM_STATE", True);
    XEvent e;
    e.xclient.type         = ClientMessage;
    e.xclient.window       = win;
    e.xclient.message_type = _NET_WM_STATE;
    e.xclient.format       = 32;
    e.xclient.data.l[0]    = (int)true_false;  // Toggle
    e.xclient.data.l[1]    = _NET_WM_STATE_FULLSCREEN;
    e.xclient.data.l[2]    = 0;
    e.xclient.data.l[3]    = 1;
    e.xclient.data.l[4]    = 0;

    XSendEvent(display->display, DefaultRootWindow(display->display), False, SubstructureRedirectMask | SubstructureNotifyMask, &e);
//    XMoveResizeWindow(display->display, win, 0, 0, windowed_size[0], windowed_size[1]);
}

void X11Window::Move(int x, int y)
{
    XMoveWindow(display->display, win, x, y);
}

void X11Window::Resize(unsigned int w, unsigned int h)
{
    XResizeWindow(display->display, win, w, h);
}

KeyModifierBitmask GetEventFlagsFromXState(unsigned int state) {
  KeyModifierBitmask flags;

  if (state & ShiftMask)
    flags |= KeyModifierShift;
  if (state & ControlMask)
    flags |= KeyModifierCtrl;
  if (state & Mod1Mask)
    flags |= KeyModifierAlt;
  if (state & Mod4Mask)
    flags |= KeyModifierCmd;
  if (state & Mod5Mask) // altgr
    flags |= KeyModifierAlt;
//  if (state & LockMask) // capslock
//  if (state & Mod2Mask) // numlock
//  if (state & Mod3Mask) // mod3
//  if (state & Button1Mask) // LEFT_MOUSE_BUTTON;
//  if (state & Button2Mask) // MIDDLE_MOUSE_BUTTON;
//  if (state & Button3Mask) // RIGHT_MOUSE_BUTTON;
  return flags;
}

void X11Window::ProcessEvents()
{
    XEvent ev;
    while(XPending(display->display) > 0)
    {
        XNextEvent(display->display, &ev);


        switch(ev.type){
        case ConfigureNotify:
            ResizeSignal(WindowResizeEvent{ev.xconfigure.width, ev.xconfigure.height});
            break;
        case ClientMessage:
            // We've only registered to receive WM_DELETE_WINDOW, so no further checks needed.
            CloseSignal();
            break;
        case ButtonPress:
        case ButtonRelease:
        {
            const int button = ev.xbutton.button-1;
            MouseSignal(MouseEvent{
               WindowInputEvent{(float)ev.xbutton.x, (float)ev.xbutton.y,
                    GetEventFlagsFromXState(ev.xkey.state)},
               button, ev.xbutton.type == ButtonPress
           });
           break;
        }
        case FocusOut:
            break;
        case MotionNotify:
            if(ev.xmotion.state & (Button1Mask|Button2Mask|Button3Mask) ) {
                MouseMotionSignal(MouseMotionEvent{
                    (float)ev.xbutton.x, (float)ev.xbutton.y,
                    GetEventFlagsFromXState(ev.xkey.state),
                });
            }else{
                PassiveMouseMotionSignal(MouseMotionEvent{
                    (float)ev.xbutton.x, (float)ev.xbutton.y,
                    GetEventFlagsFromXState(ev.xkey.state)
                });
            }
            break;
        case KeyPress:
        case KeyRelease:
            int key;
            char ch;
            KeySym sym;

            if( XLookupString(&ev.xkey,&ch,1,&sym,0) == 0) {
                switch (sym) {
                case XK_F1:        key = PANGO_SPECIAL + PANGO_KEY_F1         ; break;
                case XK_F2:        key = PANGO_SPECIAL + PANGO_KEY_F2         ; break;
                case XK_F3:        key = PANGO_SPECIAL + PANGO_KEY_F3         ; break;
                case XK_F4:        key = PANGO_SPECIAL + PANGO_KEY_F4         ; break;
                case XK_F5:        key = PANGO_SPECIAL + PANGO_KEY_F5         ; break;
                case XK_F6:        key = PANGO_SPECIAL + PANGO_KEY_F6         ; break;
                case XK_F7:        key = PANGO_SPECIAL + PANGO_KEY_F7         ; break;
                case XK_F8:        key = PANGO_SPECIAL + PANGO_KEY_F8         ; break;
                case XK_F9:        key = PANGO_SPECIAL + PANGO_KEY_F9         ; break;
                case XK_F10:       key = PANGO_SPECIAL + PANGO_KEY_F10        ; break;
                case XK_F11:       key = PANGO_SPECIAL + PANGO_KEY_F11        ; break;
                case XK_F12:       key = PANGO_SPECIAL + PANGO_KEY_F12        ; break;
                case XK_Left:      key = PANGO_SPECIAL + PANGO_KEY_LEFT       ; break;
                case XK_Up:        key = PANGO_SPECIAL + PANGO_KEY_UP         ; break;
                case XK_Right:     key = PANGO_SPECIAL + PANGO_KEY_RIGHT      ; break;
                case XK_Down:      key = PANGO_SPECIAL + PANGO_KEY_DOWN       ; break;
                case XK_Page_Up:   key = PANGO_SPECIAL + PANGO_KEY_PAGE_UP    ; break;
                case XK_Page_Down: key = PANGO_SPECIAL + PANGO_KEY_PAGE_DOWN  ; break;
                case XK_Home:      key = PANGO_SPECIAL + PANGO_KEY_HOME       ; break;
                case XK_End:       key = PANGO_SPECIAL + PANGO_KEY_END        ; break;
                case XK_Insert:    key = PANGO_SPECIAL + PANGO_KEY_INSERT     ; break;
                case XK_Shift_L:
                case XK_Shift_R:
                case XK_Control_L:
                case XK_Control_R:
                case XK_Alt_L:
                case XK_Alt_R:
                case XK_Super_L:
                case XK_Super_R:
                default:
                    key = -1;
                    break;
                }
            }else{
                key = ch;
            }

            if(key >=0) {
                KeyboardSignal(KeyboardEvent{
                    WindowInputEvent{(float)ev.xkey.x, (float)ev.xkey.y,
                        GetEventFlagsFromXState(ev.xkey.state)},
                    (unsigned char)key, ev.type == KeyPress
                });
            }

            break;
        }
    }
}

void X11Window::SwapBuffers() {
    CheckEGLDieOnError();
    EGLBoolean suc = eglSwapBuffers(glcontext->egl_display, glcontext->egl_surface);
    if(suc==EGL_FALSE) {
        std::cerr << "egl swap failed" << std::endl;
    }
    CheckEGLDieOnError();
}

std::unique_ptr<WindowInterface> CreateX11WindowAndBind(const std::string& window_title, const int w, const int h, const std::string& display_name)
{
    std::shared_ptr<X11Display> newdisplay = std::make_shared<X11Display>(display_name.empty() ? NULL : display_name.c_str() );
    if (!newdisplay) {
        throw std::runtime_error("Pangolin X11: Failed to open X display");
    }

    window_mutex.lock();
    std::shared_ptr<X11GlContext> newglcontext = std::make_shared<X11GlContext>(newdisplay);

    if(!global_gl_context.lock()) {
        global_gl_context = newglcontext;
    }
    window_mutex.unlock();

    X11Window* win = new X11Window(window_title, w, h, newdisplay, newglcontext);

    return std::unique_ptr<WindowInterface>(win);
}

PANGOLIN_REGISTER_FACTORY(X11Window)
{
  struct X11WindowFactory : public TypedFactoryInterface<WindowInterface> {
      std::map<std::string,Precedence> Schemes() const override
      {
          return {{"x11",10}, {"linux",10}, {"default",100}};
      }
      const char* Description() const override
      {
          return "Use X11 native window";
      }
      ParamSet Params() const override
      {
          return {{
              {"window_title","window","Title of application Window"},
              {"w","640","Requested window width"},
              {"h","480","Requested window height"},
              {"display_name","","The display name to open the window on"},
              {"double_buffered","true","Whether the window should be double buffered"},
              {"sample_buffers","1",""},
              {"samples","1",""},
              {PARAM_GL_PROFILE,{},"Ignored for now"},
          }};
      }

      std::unique_ptr<WindowInterface> Open(const Uri& uri) override {
          
      const std::string window_title = uri.Get<std::string>("window_title", "window");
      const int w = uri.Get<int>("w", 640);
      const int h = uri.Get<int>("h", 480);
      const std::string display_name = uri.Get<std::string>("display_name", "");
      return std::unique_ptr<WindowInterface>(CreateX11WindowAndBind(window_title, w, h, display_name));
    }
  };

  return FactoryRegistry::I()->RegisterFactory<WindowInterface>(std::make_shared<X11WindowFactory>());
}

}

