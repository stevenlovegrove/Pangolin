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

#include <iostream>
#include <sstream>
#include <string>
#include <map>

#include <pangolin/platform.h>
#include <pangolin/gl/glinclude.h>
#include <pangolin/gl/glglut.h>
#include <pangolin/gl/gldraw.h>
#include <pangolin/display/display.h>
#include <pangolin/display/display_internal.h>
#include <pangolin/handler/handler.h>
#include <pangolin/utils/simple_math.h>
#include <pangolin/utils/timer.h>
#include <pangolin/utils/type_convert.h>
#include <pangolin/image/image_io.h>

#define GLUT_KEY_ESCAPE 27
#define GLUT_KEY_TAB 9

#ifdef BUILD_PANGOLIN_VARS
  #include <pangolin/var/var.h>
#endif

#include <pangolin/compat/memory.h>

namespace pangolin
{

typedef std::map<std::string,boostd::shared_ptr<PangolinGl> > ContextMap;

// Map of active contexts
ContextMap contexts;

// Context active for current thread
__thread PangolinGl* context = 0;

PangolinGl::PangolinGl()
    : user_app(0), quit(false), mouse_state(0),activeDisplay(0)
#ifdef BUILD_PANGOLIN_VIDEO
    , record_view(0)
#endif
#ifdef HAVE_PYTHON
    , python_view(0)
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

void BindToContext(std::string name)
{
    ContextMap::iterator ic = contexts.find(name);
    
    if( ic == contexts.end() )
    {
        // Create and add if not found
        context = new PangolinGl;
        contexts[name] = boostd::shared_ptr<PangolinGl>(context);
        View& dc = context->base;
        dc.left = 0.0;
        dc.bottom = 0.0;
        dc.top = 1.0;
        dc.right = 1.0;
        dc.aspect = 0;
        dc.handler = &StaticHandler;
        context->is_fullscreen = false;
#ifdef HAVE_GLUT
        process::Resize(
                    glutGet(GLUT_WINDOW_WIDTH),
                    glutGet(GLUT_WINDOW_HEIGHT)
                    );
#else
        process::Resize(640,480);
#endif //HAVE_GLUT
    }else{
        context = ic->second.get();
    }
}

void Quit()
{
    context->quit = true;
}

bool ShouldQuit()
{
    return context->quit;
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

void RenderViews()
{
    Viewport::DisableScissor();
    DisplayBase().Render();
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
    }
#endif // BUILD_PANGOLIN_VIDEO

    // Disable scissor each frame
    Viewport::DisableScissor();

#ifdef HAVE_PYTHON
    if(context->python_view && context->python_view->IsShown()) {
        // Size to base
        context->python_view->v = DisplayBase().v;
        context->python_view->Render();
    }
#endif
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
    if( !context->python_view) {
        // Create console and let the pangolin context take ownership
        context->python_view = new ConsoleView(new PyInterpreter());
        context->named_managed_views["pangolin_console"] = context->python_view;
        context->python_view->SetFocus();
    }else{
        context->python_view->ToggleShow();
        if(context->python_view->IsShown()) {
            context->python_view->SetFocus();
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

void RegisterKeyPressCallback(int key, boostd::function<void(void)> func)
{
    context->keypress_hooks[key] = func;
}

void SaveWindowOnRender(std::string prefix)
{
    context->screen_capture.push(std::pair<std::string,Viewport>(prefix, context->base.v) );
}

void SaveFramebuffer(std::string prefix, const Viewport& v)
{
#ifndef HAVE_GLES

#ifdef HAVE_PNG
    Image<unsigned char> buffer;

    VideoPixelFormat fmt = VideoFormatFromString("RGBA");
    buffer.Alloc(v.w, v.h, v.w * fmt.bpp/8 );
    glReadBuffer(GL_BACK);
    glPixelStorei(GL_PACK_ALIGNMENT, 1); // TODO: Avoid this?
    glReadPixels(v.l, v.b, v.w, v.h, GL_RGBA, GL_UNSIGNED_BYTE, buffer.ptr );
    SaveImage(buffer, fmt, prefix + ".png", false);
    buffer.Dealloc();
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
    
    const int ticks = (int)TimeNow_s();
    if( ticks % 2 )
    {
        v.ActivatePixelOrthographic();
        // now, render a little red "recording" dot
        glPushAttrib(GL_ENABLE_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        const float r = 7;
        glColor3ub( 255, 0, 0 );
        glDrawCircle( v.w-2*r, v.h-2*r, r );
        glPopAttrib();
    }
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
    
    if( key == GLUT_KEY_ESCAPE) {
        context->quit = true;
    }
#ifdef HAVE_PYTHON
    // TODO: Use keypress_hook system to allow customisation
    else if( key == '`') {
        ToggleConsole();
    }
#endif
//    else if( key == GLUT_KEY_TAB) {
//        ToggleFullscreen();
//    }
    else if(context->keypress_hooks.find(key) != context->keypress_hooks.end() ) {
        context->keypress_hooks[key]();
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

    const MouseButton button = (MouseButton)(1 << (button_raw&0x7) );
    const bool pressed = (state == 0);
    
    context->had_input = context->is_double_buffered ? 2 : 1;
    
    const bool fresh_input = (context->mouse_state == 0);
    
    if( pressed ) {
        context->mouse_state |= button;
    }else{
        context->mouse_state &= ~button;
    }
    
#ifdef HAVE_GLUT
    context->mouse_state &= 0x0000ffff;
    context->mouse_state |= glutGetModifiers() << 16;
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
#ifdef HAVE_GLUT
    context->mouse_state &= 0x0000ffff;
    context->mouse_state |= glutGetModifiers() << 16;
#endif    
    
    SpecialInput(InputSpecialScroll, last_x, last_y, x, y, 0, 0);
}

void Zoom(float m)
{
#ifdef HAVE_GLUT
    context->mouse_state &= 0x0000ffff;
    context->mouse_state |= glutGetModifiers() << 16;
#endif    
    
    SpecialInput(InputSpecialZoom, last_x, last_y, m, 0, 0, 0);
}

void Rotate(float r)
{
#ifdef HAVE_GLUT
    context->mouse_state &= 0x0000ffff;
    context->mouse_state |= glutGetModifiers() << 16;
#endif    
    
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

void PangolinCommonInit()
{
}


}
