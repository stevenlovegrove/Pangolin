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
// https://www.opengl.org/wiki/Tutorial:_OpenGL_3.0_Context_Creation_%28GLX%29

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

#include <GL/glx.h>

namespace pangolin
{

std::mutex window_mutex;
std::weak_ptr<X11GlContext> global_gl_context;

const long EVENT_MASKS = ButtonPressMask|ButtonReleaseMask|StructureNotifyMask|ButtonMotionMask|PointerMotionMask|KeyPressMask|KeyReleaseMask|FocusChangeMask;

#define GLX_CONTEXT_MAJOR_VERSION_ARB       0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB       0x2092
typedef GLXContext (*glXCreateContextAttribsARBProc)(::Display*, ::GLXFBConfig, ::GLXContext, Bool, const int*);

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

::GLXFBConfig ChooseFrameBuffer(
    ::Display *display, bool glx_doublebuffer,
    int glx_sample_buffers, int glx_samples
) {
    // Desired attributes
    int visual_attribs[] =
    {
        GLX_X_RENDERABLE    , True,
        GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
        GLX_RENDER_TYPE     , GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
        GLX_RED_SIZE        , 8,
        GLX_GREEN_SIZE      , 8,
        GLX_BLUE_SIZE       , 8,
        GLX_ALPHA_SIZE      , 8,
        GLX_DEPTH_SIZE      , 24,
        GLX_STENCIL_SIZE    , 8,
        GLX_DOUBLEBUFFER    , glx_doublebuffer ? True : False,
        None
    };

    int fbcount;
    GLXFBConfig* fbc = glXChooseFBConfig(display, DefaultScreen(display), visual_attribs, &fbcount);
    if (!fbc) {
        throw std::runtime_error("Pangolin X11: Unable to retrieve framebuffer options");
    }

    int best_fbc = -1;
    int worst_fbc = -1;
    int best_num_samp = -1;
    int worst_num_samp = 999;

    // Enumerate framebuffer options, storing the best and worst that match our attribs
    for (int i=0; i<fbcount; ++i)
    {
        XVisualInfo *vi = glXGetVisualFromFBConfig( display, fbc[i] );
        if ( vi )
        {
            int samp_buf, samples;
            glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf );
            glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLES       , &samples  );

            // Filter for the best available.
            if ( samples > best_num_samp ) {
                best_fbc = i;
                best_num_samp = samples;
            }

            // Filter lowest settings which match minimum user requirement.
            if ( samp_buf >= glx_sample_buffers && samples >= glx_samples && samples < worst_num_samp ) {
                worst_fbc = i;
                worst_num_samp = samples;
            }
        }
        XFree( vi );
    }

    // Select the minimum suitable option. The 'best' is often too slow.
    int chosen_fbc_id = worst_fbc;

    // If minimum requested isn't available, return the best that is.
    if(chosen_fbc_id < 0) {
        pango_print_warn("Framebuffer with requested attributes not available. Using available framebuffer. You may see visual artifacts.");
        chosen_fbc_id = best_fbc;
    }

    ::GLXFBConfig chosenFbc = fbc[ chosen_fbc_id ];
    XFree( fbc );
    return chosenFbc;
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

GLXContext CreateGlContext(::Display *display, ::GLXFBConfig chosenFbc, GLXContext share_context = 0)
{
    int glx_major, glx_minor;
    if ( !glXQueryVersion( display, &glx_major, &glx_minor ) ||
         ( ( glx_major == 1 ) && ( glx_minor < 3 ) ) || ( glx_major < 1 ) )
    {
        throw std::runtime_error("Pangolin X11: Invalid GLX version. Require GLX >= 1.3");
    }

    GLXContext new_ctx;

    // Get the default screen's GLX extension list
    const char *glxExts = glXQueryExtensionsString( display, DefaultScreen( display ) );

    glXCreateContextAttribsARBProc glXCreateContextAttribsARB =
            (glXCreateContextAttribsARBProc) glXGetProcAddressARB(
                (const GLubyte *) "glXCreateContextAttribsARB"
            );

    // Install an X error handler so the application won't exit if GL 3.0
    // context allocation fails. Handler is global and shared across all threads.
    ctxErrorOccurred = false;
    int (*oldHandler)(::Display*, ::XErrorEvent*) = XSetErrorHandler(&ctxErrorHandler);

    if ( isExtensionSupported( glxExts, "GLX_ARB_create_context" ) && glXCreateContextAttribsARB )
    {
        int context_attribs[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
            GLX_CONTEXT_MINOR_VERSION_ARB, 0,
            //GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            None
        };

        new_ctx = glXCreateContextAttribsARB( display, chosenFbc, share_context, True, context_attribs );

        // Sync to ensure any errors generated are processed.
        XSync( display, False );
        if ( ctxErrorOccurred || !new_ctx ) {
            ctxErrorOccurred = false;
            // Fall back to old-style 2.x context. Implementations will return the newest
            // context version compatible with OpenGL versions less than version 3.0.
            context_attribs[1] = 1;  // GLX_CONTEXT_MAJOR_VERSION_ARB = 1
            context_attribs[3] = 0;  // GLX_CONTEXT_MINOR_VERSION_ARB = 0
            new_ctx = glXCreateContextAttribsARB( display, chosenFbc, share_context, True, context_attribs );
        }
    } else {
        // Fallback to GLX 1.3 Context
        new_ctx = glXCreateNewContext( display, chosenFbc, GLX_RGBA_TYPE, share_context, True );
    }

    // Sync to ensure any errors generated are processed.
    XSync( display, False );

    // Restore the original error handler
    XSetErrorHandler( oldHandler );

    if ( ctxErrorOccurred || !new_ctx ) {
        throw std::runtime_error("Pangolin X11: Failed to create an OpenGL context");
    }

    // Verifying that context is a direct context
    if ( ! glXIsDirect ( display, new_ctx ) ) {
        pango_print_warn("Pangolin X11: Indirect GLX rendering context obtained\n");
    }

    return new_ctx;
}

X11GlContext::X11GlContext(std::shared_ptr<X11Display>& d, ::GLXFBConfig chosenFbc, std::shared_ptr<X11GlContext> shared_context)
    : display(d), shared_context(shared_context)
{
    // prevent chained sharing
    while(shared_context && shared_context->shared_context) {
        shared_context = shared_context->shared_context;
    }

    // Contexts can't be shared across different displays.
    if(shared_context && shared_context->display != d) {
        shared_context.reset();
    }

    glcontext = CreateGlContext(display->display, chosenFbc, shared_context ? shared_context->glcontext : 0);
}

X11GlContext::~X11GlContext()
{
    glXDestroyContext( display->display, glcontext );
}

X11Window::X11Window(
    const std::string& title, int width, int height,
    std::shared_ptr<X11Display>& display, ::GLXFBConfig chosenFbc
) : display(display), glcontext(0), win(0), cmap(0)
{
    // Get a visual
    XVisualInfo *vi = glXGetVisualFromFBConfig( display->display, chosenFbc );

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

    XFree( vi );

    if ( !win ) {
        throw std::runtime_error("Pangolin X11: Failed to create window." );
    }

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
}

X11Window::~X11Window()
{
    glXMakeCurrent( display->display, 0, 0 );
    XDestroyWindow( display->display, win );
    XFreeColormap( display->display, cmap );
}

void X11Window::MakeCurrent(GLXContext ctx)
{
    glXMakeCurrent( display->display, win, ctx );
}

void X11Window::MakeCurrent()
{
    MakeCurrent(glcontext ? glcontext->glcontext : global_gl_context.lock()->glcontext);
}

void X11Window::RemoveCurrent()
{
    glXMakeCurrent(display->display, 0, nullptr);
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
               (float)ev.xbutton.x, (float)ev.xbutton.y,
               GetEventFlagsFromXState(ev.xkey.state),
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
                    (float)ev.xkey.x, (float)ev.xkey.y,
                    GetEventFlagsFromXState(ev.xkey.state),
                    (unsigned char)key, ev.type == KeyPress
                });
            }

            break;
        }
    }
}

void X11Window::SwapBuffers() {
    glXSwapBuffers(display->display, win);
}

std::unique_ptr<WindowInterface> CreateX11WindowAndBind(const std::string& window_title, const int w, const int h, const std::string& display_name, const bool double_buffered, const int  sample_buffers, const int  samples)
{
    std::shared_ptr<X11Display> newdisplay = std::make_shared<X11Display>(display_name.empty() ? NULL : display_name.c_str() );
    if (!newdisplay) {
        throw std::runtime_error("Pangolin X11: Failed to open X display");
    }
    ::GLXFBConfig newfbc = ChooseFrameBuffer(newdisplay->display, double_buffered, sample_buffers, samples);

    window_mutex.lock();
    std::shared_ptr<X11GlContext> newglcontext = std::make_shared<X11GlContext>(
        newdisplay, newfbc, global_gl_context.lock()
    );

    if(!global_gl_context.lock()) {
        global_gl_context = newglcontext;
    }
    window_mutex.unlock();

    X11Window* win = new X11Window(window_title, w, h, newdisplay, newfbc);
    win->glcontext = newglcontext;

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
              {PARAM_GL_PROFILE,"Ignored for now"},
          }};
      }

      std::unique_ptr<WindowInterface> Open(const Uri& uri) override {
          
      const std::string window_title = uri.Get<std::string>("window_title", "window");
      const int w = uri.Get<int>("w", 640);
      const int h = uri.Get<int>("h", 480);
      const std::string display_name = uri.Get<std::string>("display_name", "");
      const bool double_buffered = uri.Get<bool>("double_buffered", true);
      const int sample_buffers = uri.Get<int>("sample_buffers", 1);
      const int samples = uri.Get<int>("samples", 1);
      return std::unique_ptr<WindowInterface>(CreateX11WindowAndBind(window_title, w, h, display_name, double_buffered, sample_buffers, samples));
    }
  };

  return FactoryRegistry::I()->RegisterFactory<WindowInterface>(std::make_shared<X11WindowFactory>());
}

}

