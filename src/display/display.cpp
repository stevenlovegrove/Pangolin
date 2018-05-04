/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011 Steven Lovegrove, Richard Newcombe
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

#include <pangolin/platform.h>

#ifdef HAVE_PYTHON
#include <pangolin/python/pyinterpreter.h>
#include <pangolin/console/ConsoleView.h>
#endif // HAVE_PYTHON

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <mutex>
#include <cstdlib>

#include <pangolin/factory/factory_registry.h>
#include <pangolin/window_frameworks.h>
#include <pangolin/gl/glinclude.h>
#include <pangolin/gl/gldraw.h>
#include <pangolin/display/display.h>
#include <pangolin/display/display_internal.h>
#include <pangolin/handler/handler.h>
#include <pangolin/utils/simple_math.h>
#include <pangolin/utils/timer.h>
#include <pangolin/utils/type_convert.h>
#include <pangolin/image/image_io.h>

#ifdef BUILD_PANGOLIN_VARS
  #include <pangolin/var/var.h>
#endif

namespace pangolin
{

#ifdef BUILD_PANGOLIN_VIDEO
  // Forward declaration.
  void SaveFramebuffer(VideoOutput& video, const Viewport& v);
#endif // BUILD_PANGOLIN_VIDEO

const char* PARAM_DISPLAYNAME    = "DISPLAYNAME";
const char* PARAM_DOUBLEBUFFER   = "DOUBLEBUFFER";
const char* PARAM_SAMPLE_BUFFERS = "SAMPLE_BUFFERS";
const char* PARAM_SAMPLES        = "SAMPLES";
const char* PARAM_HIGHRES        = "HIGHRES";


typedef std::map<std::string,std::shared_ptr<PangolinGl> > ContextMap;

// Map of active contexts
ContextMap contexts;
std::mutex contexts_mutex;
bool one_time_window_frameworks_init = false;

// Context active for current thread
__thread PangolinGl* context = 0;

PangolinGl::PangolinGl()
    : user_app(0), is_high_res(false), quit(false), mouse_state(0),activeDisplay(0)
#ifdef BUILD_PANGOLIN_VIDEO
    , record_view(0)
#endif
#ifdef HAVE_PYTHON
    , console_view(0)
#endif
{
}

PangolinGl::~PangolinGl()
{
    // Free displays owned by named_managed_views
    for(ViewMap::iterator iv = named_managed_views.begin(); iv != named_managed_views.end(); ++iv) {
        delete iv->second;
    }
    named_managed_views.clear();
}

PangolinGl* GetCurrentContext()
{
    return context;
}

PangolinGl *FindContext(const std::string& name)
{
    contexts_mutex.lock();
    ContextMap::iterator ic = contexts.find(name);
    PangolinGl* context = (ic == contexts.end()) ? 0 : ic->second.get();
    contexts_mutex.unlock();
    return context;
}

WindowInterface& CreateWindowAndBind(std::string window_title, int w, int h, const Params& params)
{
    std::unique_lock<std::mutex> l(contexts_mutex);

    if(!one_time_window_frameworks_init) {
        one_time_window_frameworks_init = LoadBuiltInWindowFrameworks();
    }

    pangolin::Uri win_uri;

    if(const char* extra_params = std::getenv("PANGOLIN_WINDOW_URI"))
    {
        // Take any defaults from the environment
        win_uri = pangolin::ParseUri(extra_params);
    }

    // Override with anything the program specified
    win_uri.Set("w", w);
    win_uri.Set("h", h);
    win_uri.Set("window_title", window_title);
    win_uri.params.insert(std::end(win_uri.params), std::begin(params.params), std::end(params.params));

    // Fall back to default scheme if non specified.
    if(win_uri.scheme.empty()) {
#if defined(_LINUX_)
      win_uri.scheme = "linux";
#elif defined(_WIN_)
      win_uri.scheme = "winapi";
#elif defined(_OSX_)
      win_uri.scheme = "cocoa";
#else
#     error "No default window api for this platform."
#endif
    }

    std::unique_ptr<WindowInterface> window = FactoryRegistry<WindowInterface>::I().Open(win_uri);

    // We're expecting not only a WindowInterface, but a PangolinGl.
    if(!window || !dynamic_cast<PangolinGl*>(window.get())) {
        throw WindowExceptionNoKnownHandler(win_uri.scheme);
    }

    std::shared_ptr<PangolinGl> context(dynamic_cast<PangolinGl*>(window.release()));
    RegisterNewContext(window_title, context );
    // is_high_res will alter default font size and a few other gui elements.
    context->is_high_res = win_uri.Get(PARAM_HIGHRES,false);
    context->MakeCurrent();
    context->ProcessEvents();
    glewInit();

    return *context;
}

// Assumption: unique lock is held on contexts_mutex for multi-threaded operation
void RegisterNewContext(const std::string& name, std::shared_ptr<PangolinGl> newcontext)
{
    // Set defaults
    newcontext->base.left = 0.0;
    newcontext->base.bottom = 0.0;
    newcontext->base.top = 1.0;
    newcontext->base.right = 1.0;
    newcontext->base.aspect = 0;
    newcontext->base.handler = &StaticHandler;
    newcontext->is_fullscreen = false;

    // Create and add
    if( contexts.find(name) != contexts.end() ) {
        throw std::runtime_error("Context already exists.");
    }
    contexts[name] = newcontext;

    // Process the following as if this context is now current.
    PangolinGl *oldContext = context;
    context = newcontext.get();
    process::Resize(
        newcontext->windowed_size[0],
        newcontext->windowed_size[1]
    );

    // Default key bindings can be overridden
    RegisterKeyPressCallback(PANGO_KEY_ESCAPE, Quit );
    RegisterKeyPressCallback('\t', ToggleFullscreen );
    RegisterKeyPressCallback('`',  ToggleConsole );

    context = oldContext;
}

WindowInterface* GetBoundWindow()
{
    return context;
}

void DestroyWindow(const std::string& name)
{
    contexts_mutex.lock();
    ContextMap::iterator ic = contexts.find(name);
    PangolinGl *context_to_destroy = (ic == contexts.end()) ? 0 : ic->second.get();
    if (context_to_destroy == context) {
        context = nullptr;
    }
    size_t erased = contexts.erase(name);
    if(erased == 0) {
        pango_print_warn("Context '%s' doesn't exist for deletion.\n", name.c_str());
    }
    contexts_mutex.unlock();
}

WindowInterface& BindToContext(std::string name)
{
    std::unique_lock<std::mutex> l(contexts_mutex);

    // N.B. context is modified prior to invoking MakeCurrent so that
    // state management callbacks (such as Resize()) can be correctly
    // processed.
    PangolinGl *context_to_bind = FindContext(name);
    if( !context_to_bind )
    {
        std::shared_ptr<PangolinGl> newcontext(new PangolinGl());
        RegisterNewContext(name, newcontext);
        newcontext->MakeCurrent();
        return *(newcontext.get());
    }else{
        context_to_bind->MakeCurrent();
        return *context_to_bind;
    }
}

void Quit()
{
    context->quit = true;
}

void QuitAll()
{
    for(auto nc : contexts) {
        nc.second->quit = true;
    }
}

bool ShouldQuit()
{
    return !context || context->quit;
}

bool HadInput()
{
    if( context->had_input > 0 )
    {
        --context->had_input;
        return true;
    }
    return false;
}

bool HasResized()
{
    if( context->has_resized > 0 )
    {
        --context->has_resized;
        return true;
    }
    return false;
}

void StartFullScreen() {
    if(!context->is_fullscreen) {
        context->ToggleFullscreen();
        context->is_fullscreen = true;
    }
}

void StopFullScreen() {
    if(context->is_fullscreen) {
        context->ToggleFullscreen();
        context->is_fullscreen = false;
    }
}

void SetFullscreen(bool fullscreen)
{
    if(fullscreen) {
        StartFullScreen();
    }else{
        StopFullScreen();
    }
}

void RenderViews()
{
    Viewport::DisableScissor();
    DisplayBase().Render();
}

void RenderRecordGraphic(const Viewport& v)
{
    const float r = 7;
    v.ActivatePixelOrthographic();
    glRecordGraphic(v.w-2*r, v.h-2*r, r);
}

void PostRender()
{
    while(context->screen_capture.size()) {
        std::pair<std::string,Viewport> fv = context->screen_capture.front();
        context->screen_capture.pop();
        SaveFramebuffer(fv.first, fv.second);
    }
    
#ifdef BUILD_PANGOLIN_VIDEO
    if(context->recorder.IsOpen()) {
        SaveFramebuffer(context->recorder, context->record_view->GetBounds() );
        RenderRecordGraphic(context->record_view->GetBounds());
    }
#endif // BUILD_PANGOLIN_VIDEO

    // Disable scissor each frame
    Viewport::DisableScissor();
}

void FinishFrame()
{
    RenderViews();
    PostRender();
    context->SwapBuffers();
    context->ProcessEvents();
}

View& DisplayBase()
{
    return context->base;
}

View& CreateDisplay()
{
    int iguid = rand();
    std::stringstream ssguid;
    ssguid << iguid;
    return Display(ssguid.str());
}

void ToggleConsole()
{
#ifdef HAVE_PYTHON
    if( !context->console_view) {
        // Create console and let the pangolin context take ownership
        context->console_view = new ConsoleView(new PyInterpreter());
        context->named_managed_views["pangolin_console"] = context->console_view;
        context->console_view->SetFocus();
        context->console_view->zorder = std::numeric_limits<int>::max();
        DisplayBase().AddDisplay(*context->console_view);
    }else{
        context->console_view->ToggleShow();
        if(context->console_view->IsShown()) {
            context->console_view->SetFocus();
        }
    }
#endif
}

void ToggleFullscreen()
{
    SetFullscreen(!context->is_fullscreen);
}

View& Display(const std::string& name)
{
    // Get / Create View
    ViewMap::iterator vi = context->named_managed_views.find(name);
    if( vi != context->named_managed_views.end() )
    {
        return *(vi->second);
    }else{
        View * v = new View();
        context->named_managed_views[name] = v;
        v->handler = &StaticHandler;
        context->base.views.push_back(v);
        return *v;
    }
}

void RegisterKeyPressCallback(int key, std::function<void(void)> func)
{
    context->keypress_hooks[key] = func;
}

void SaveWindowOnRender(std::string prefix)
{
    context->screen_capture.push(std::pair<std::string,Viewport>(prefix, context->base.v) );
}

void SaveFramebuffer(std::string prefix, const Viewport& v)
{
    PANGOLIN_UNUSED(prefix);
    PANGOLIN_UNUSED(v);
    
#ifndef HAVE_GLES

#ifdef HAVE_PNG
    PixelFormat fmt = PixelFormatFromString("RGBA32");
    TypedImage buffer(v.w, v.h, fmt );
    glReadBuffer(GL_BACK);
    glPixelStorei(GL_PACK_ALIGNMENT, 1); // TODO: Avoid this?
    glReadPixels(v.l, v.b, v.w, v.h, GL_RGBA, GL_UNSIGNED_BYTE, buffer.ptr );
    SaveImage(buffer, fmt, prefix + ".png", false);
#endif // HAVE_PNG
    
#endif // HAVE_GLES
}

#ifdef BUILD_PANGOLIN_VIDEO
void SaveFramebuffer(VideoOutput& video, const Viewport& v)
{
#ifndef HAVE_GLES    
    const StreamInfo& si = video.Streams()[0];
    if(video.Streams().size()==0 || (int)si.Width() != v.w || (int)si.Height() != v.h) {
        video.Close();
        return;
    }

    static basetime last_time = TimeNow();
    const basetime time_now = TimeNow();
    last_time = time_now;
    
    static std::vector<unsigned char> img;
    img.resize(v.w*v.h*4);

    glReadBuffer(GL_BACK);
    glPixelStorei(GL_PACK_ALIGNMENT, 1); // TODO: Avoid this?
    glReadPixels(v.l, v.b, v.w, v.h, GL_RGB, GL_UNSIGNED_BYTE, &img[0] );
    video.WriteStreams(&img[0]);
#endif // HAVE_GLES
}
#endif // BUILD_PANGOLIN_VIDEO


namespace process
{
float last_x = 0;
float last_y = 0;

void Keyboard( unsigned char key, int x, int y)
{
    // Force coords to match OpenGl Window Coords
    y = context->base.v.h - y;
    
#ifdef HAVE_APPLE_OPENGL_FRAMEWORK
    // Switch backspace and delete for OSX!
    if(key== '\b') {
        key = 127;
    }else if(key == 127) {
        key = '\b';
    }
#endif

    context->had_input = context->is_double_buffered ? 2 : 1;

    // Check if global key hook exists
    const KeyhookMap::iterator hook = context->keypress_hooks.find(key);
    
#ifdef HAVE_PYTHON
    // Console receives all input when it is open
    if( context->console_view && context->console_view->IsShown() ) {
        context->console_view->Keyboard(*(context->console_view),key,x,y,true);
    }else
#endif
    if(hook != context->keypress_hooks.end() ) {
        hook->second();
    } else if(context->activeDisplay && context->activeDisplay->handler) {
        context->activeDisplay->handler->Keyboard(*(context->activeDisplay),key,x,y,true);
    }
}

void KeyboardUp(unsigned char key, int x, int y)
{
    // Force coords to match OpenGl Window Coords
    y = context->base.v.h - y;
    
    if(context->activeDisplay && context->activeDisplay->handler)
    {
        context->activeDisplay->handler->Keyboard(*(context->activeDisplay),key,x,y,false);
    }
}

void SpecialFunc(int key, int x, int y)
{
    Keyboard(key+128,x,y);
}

void SpecialFuncUp(int key, int x, int y)
{
    KeyboardUp(key+128,x,y);
}


void Mouse( int button_raw, int state, int x, int y)
{
    // Force coords to match OpenGl Window Coords
    y = context->base.v.h - y;
    
    last_x = (float)x;
    last_y = (float)y;

    const MouseButton button = (MouseButton)(1 << (button_raw & 0xf) );
    const bool pressed = (state == 0);
    
    context->had_input = context->is_double_buffered ? 2 : 1;
    
    const bool fresh_input = ( (context->mouse_state & 7) == 0);
    
    if( pressed ) {
        context->mouse_state |= (button&7);
    }else{
        context->mouse_state &= ~(button&7);
    }
    
#if defined(_WIN_)
    context->mouse_state &= 0x0000ffff;
    context->mouse_state |= (button_raw >> 4) << 16;
#endif
    
    if(fresh_input) {
        context->base.handler->Mouse(context->base,button,x,y,pressed,context->mouse_state);
    }else if(context->activeDisplay && context->activeDisplay->handler) {
        context->activeDisplay->handler->Mouse(*(context->activeDisplay),button,x,y,pressed,context->mouse_state);
    }
}

void MouseMotion( int x, int y)
{
    // Force coords to match OpenGl Window Coords
    y = context->base.v.h - y;
    
    last_x = (float)x;
    last_y = (float)y;
    
    context->had_input = context->is_double_buffered ? 2 : 1;
    
    if( context->activeDisplay)
    {
        if( context->activeDisplay->handler )
            context->activeDisplay->handler->MouseMotion(*(context->activeDisplay),x,y,context->mouse_state);
    }else{
        context->base.handler->MouseMotion(context->base,x,y,context->mouse_state);
    }
}

void PassiveMouseMotion(int x, int y)
{
    // Force coords to match OpenGl Window Coords
    y = context->base.v.h - y;
    
    context->base.handler->PassiveMouseMotion(context->base,x,y,context->mouse_state);
    
    last_x = (float)x;
    last_y = (float)y;
}

void Display()
{
    // No implementation
}

void Resize( int width, int height )
{
    if( !context->is_fullscreen )
    {
        context->windowed_size[0] = width;
        context->windowed_size[1] = height;
    }
    // TODO: Fancy display managers seem to cause this to mess up?
    context->had_input = 20; //context->is_double_buffered ? 2 : 1;
    context->has_resized = 20; //context->is_double_buffered ? 2 : 1;
    Viewport win(0,0,width,height);
    context->base.Resize(win);
}

void SpecialInput(InputSpecial inType, float x, float y, float p1, float p2, float p3, float p4)
{
    // Assume coords already match OpenGl Window Coords

    context->had_input = context->is_double_buffered ? 2 : 1;
    
    const bool fresh_input = (context->mouse_state == 0);
    
    if(fresh_input) {
        context->base.handler->Special(context->base,inType,x,y,p1,p2,p3,p4,context->mouse_state);
    }else if(context->activeDisplay && context->activeDisplay->handler) {
        context->activeDisplay->handler->Special(*(context->activeDisplay),inType,x,y,p1,p2,p3,p4,context->mouse_state);
    }
}

void Scroll(float x, float y)
{
    SpecialInput(InputSpecialScroll, last_x, last_y, x, y, 0, 0);
}

void Zoom(float m)
{
    SpecialInput(InputSpecialZoom, last_x, last_y, m, 0, 0, 0);
}

void Rotate(float r)
{
    SpecialInput(InputSpecialRotate, last_x, last_y, r, 0, 0, 0);
}

void SubpixMotion(float x, float y, float pressure, float rotation, float tiltx, float tilty)
{
    // Force coords to match OpenGl Window Coords
    y = context->base.v.h - y;
    SpecialInput(InputSpecialTablet, x, y, pressure, rotation, tiltx, tilty);
}
}

void DrawTextureToViewport(GLuint texid)
{
    OpenGlRenderState::ApplyIdentity();
    glBindTexture(GL_TEXTURE_2D, texid);
    glEnable(GL_TEXTURE_2D);
    
    GLfloat sq_vert[] = { -1,-1,  1,-1,  1, 1,  -1, 1 };
    glVertexPointer(2, GL_FLOAT, 0, sq_vert);
    glEnableClientState(GL_VERTEX_ARRAY);   

    GLfloat sq_tex[]  = { 0,0,  1,0,  1,1,  0,1  };
    glTexCoordPointer(2, GL_FLOAT, 0, sq_tex);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
         
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glDisable(GL_TEXTURE_2D);
}

ToggleViewFunctor::ToggleViewFunctor(View& view)
    : view(view)
{
}

ToggleViewFunctor::ToggleViewFunctor(const std::string& name)
    : view(Display(name))
{
}

void ToggleViewFunctor::operator()()
{
    view.ToggleShow();
}

}
