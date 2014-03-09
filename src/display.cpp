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
#include <pangolin/glinclude.h>
#include <pangolin/display.h>
#include <pangolin/display_internal.h>
#include <pangolin/simple_math.h>
#include <pangolin/timer.h>
#include <pangolin/type_convert.h>

#define GLUT_KEY_ESCAPE 27
#define GLUT_KEY_TAB 9

#ifdef BUILD_PANGOLIN_VARS
  #include <pangolin/vars.h>
#endif

#ifdef HAVE_BOOST_GIL
    #include <boost/gil/gil_all.hpp>
    #ifdef HAVE_PNG
    #define png_infopp_NULL (png_infopp)NULL
    #define int_p_NULL (int*)NULL
    #include <boost/gil/extension/io/png_io.hpp>
    #endif // HAVE_PNG
    #ifdef HAVE_JPEG
    #include <boost/gil/extension/io/jpeg_io.hpp>
    #endif // HAVE_JPEG
    #ifdef HAVE_TIFF
    #include <boost/gil/extension/io/tiff_io.hpp>
    #endif // HAVE_TIFF
#endif // HAVE_BOOST_GIL

#include <pangolin/compat/memory.h>

namespace pangolin
{

typedef std::map<std::string,boostd::shared_ptr<PangolinGl> > ContextMap;

// Map of active contexts
ContextMap contexts;

// Context active for current thread
__thread PangolinGl* context = 0;

PangolinGl::PangolinGl()
    : user_app(0), quit(false), mouse_state(0), activeDisplay(0)
{
#if defined(HAVE_GLCONSOLE) && defined(HAVE_GLES)
    console.m_fOverlayPercent = 0.5;
#endif    
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
#ifdef HAVE_GLUT
    return context->quit || !glutGetWindow();
#else
    return context->quit;
#endif
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
#ifdef HAVE_BOOST_GIL
    while(context->screen_capture.size()) {
        std::pair<std::string,Viewport> fv = context->screen_capture.front();
        context->screen_capture.pop();
        SaveFramebuffer(fv.first, fv.second);
    }
#endif // HAVE_BOOST_GIL
    
#ifdef BUILD_PANGOLIN_VIDEO
    if(context->recorder.IsOpen()) {
        SaveFramebuffer(context->recorder, context->record_view->GetBounds() );
    }
#endif // BUILD_PANGOLIN_VIDEO

    DisplayBase().Activate();
    Viewport::DisableScissor();
    
#ifdef HAVE_GLCONSOLE
    context->console.RenderConsole();
#endif // HAVE_GLCONSOLE
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
#ifdef HAVE_BOOST_GIL
    // Save colour channels
    boost::gil::rgba8_image_t img(v.w, v.h);
    glReadBuffer(GL_BACK);    
    glPixelStorei(GL_PACK_ALIGNMENT, 1); // TODO: Avoid this?    
    glReadPixels(v.l, v.b, v.w, v.h, GL_RGBA, GL_UNSIGNED_BYTE, boost::gil::interleaved_view_get_raw_data( boost::gil::view( img ) ) );
#ifdef HAVE_PNG
    boost::gil::png_write_view(prefix + ".png", flipped_up_down_view( boost::gil::const_view(img)) );
#endif // HAVE_PNG
    
    //      // Save depth channel
    //      boost::gil::gray32f_image_t depth(v.w, v.h);
    //      glReadPixels(v.l, v.b, v.w, v.h, GL_DEPTH_COMPONENT, GL_FLOAT, boost::gil::interleaved_view_get_raw_data( view( depth ) ));
    //      boost::gil::tiff_write_view(prefix + "_depth.tiff", flipped_up_down_view(const_view(depth)) );
#endif // HAVE_BOOST_GIL
#endif // HAVE_GLES
}

#ifdef BUILD_PANGOLIN_VIDEO
void SaveFramebuffer(VideoOutput& video, const Viewport& v)
{
#ifndef HAVE_GLES    
    static basetime last_time = TimeNow();
    const basetime time_now = TimeNow();
    
    if(TimeDiff_s(last_time,time_now) > video[0].BaseFrameTime() ) {
        last_time = time_now;
        unsigned char* img = new unsigned char[v.w*v.h*4];
        glReadBuffer(GL_BACK);
        glPixelStorei(GL_PACK_ALIGNMENT, 1); // TODO: Avoid this?        
        glReadPixels(v.l, v.b, v.w, v.h, GL_RGB, GL_UNSIGNED_BYTE, img );
        video[0].WriteImage(img, v.w, -v.h, "RGB24" );
        delete[] img;
    }
    
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

#ifdef HAVE_CVARS
// Pangolin CVar function hooks

bool CVarViewList( std::vector<std::string>* args )
{
#ifdef HAVE_GLCONSOLE
    std::stringstream ss;
    for(ViewMap::iterator vi = context->named_managed_views.begin();
        vi != context->named_managed_views.end(); ++vi)
    {
        ss << "'" << vi->first << "' " << std::endl;
    }
    context->console.EnterLogLine(ss.str().c_str());
#endif //HAVE_GLCONSOLE
    return true;
}

bool CVarViewShowHide( std::vector<std::string>* args )
{
    if(args && args->size() == 1) {
        Display(args->at(0)).ToggleShow();
    }else{
#ifdef HAVE_GLCONSOLE
        context->console.EnterLogLine("USAGE: pango.view.showhide view_name", LINEPROP_ERROR);        
#endif //HAVE_GLCONSOLE
    }
    return true;
}

// Pangolin CVar function hooks
bool CVarScreencap( std::vector<std::string>* args )
{
    if(args && args->size() > 0) {
        const std::string file_prefix = args->at(0);
        float scale = 1.0f;
        View* view = &DisplayBase();
        
        if(args->size() > 1)  scale = Convert<float,std::string>::Do(args->at(1));
        if(args->size() > 2)  view = &Display(args->at(2));

        if(scale == 1.0f) {
            view->SaveOnRender(file_prefix);
        }else{
            view->SaveRenderNow(file_prefix, scale);
        }

#ifdef HAVE_GLCONSOLE
        context->console.EnterLogLine("done.");
#endif // HAVE_GLCONSOLE
    }else{
#ifdef HAVE_GLCONSOLE
        context->console.EnterLogLine("USAGE: pango.screencap file_prefix [scale=1] [view_name]", LINEPROP_ERROR);
        context->console.EnterLogLine("   eg: pango.screencap my_shot", LINEPROP_ERROR);
#endif // HAVE_GLCONSOLE
    }
    return false;
}

#ifdef BUILD_PANGOLIN_VIDEO
bool CVarRecordStart( std::vector<std::string>* args )
{
    if(args && args->size() > 0) {
        const std::string uri = args->at(0);
        View* view = &DisplayBase();
        
        if(args->size() > 1) {
            view = &Display(args->at(1));
        }
        
        try {
            view->RecordOnRender(uri);
#ifdef HAVE_GLCONSOLE
            context->console.ToggleConsole();
#endif // HAVE_GLCONSOLE
            return true;
        }catch(VideoException e) {
#ifdef HAVE_GLCONSOLE
            context->console.EnterLogLine(e.what(), LINEPROP_ERROR );
#endif // HAVE_GLCONSOLE
        }
    }else{
#ifdef HAVE_GLCONSOLE
        context->console.EnterLogLine("USAGE: pango.record.start uri [view_name]", LINEPROP_ERROR);
        context->console.EnterLogLine("   eg: pango.record.start ffmpeg://screencap.avi", LINEPROP_ERROR);
#endif // HAVE_GLCONSOLE
    }
    return false;
}

bool CVarRecordStop( std::vector<std::string>* args )
{
    context->recorder.Reset();
    return true;
}
#endif // BUILD_PANGOLIN_VIDEO

#ifdef BUILD_PANGOLIN_VARS
void NewVarForCVars(void* /*data*/, const std::string& name, _Var& var, const char* /*orig_typeidname*/, bool brand_new)
{
    if(brand_new) {
        // CVars can't save names containing spaces, so map to '_' instead
        std::string cvar_name = name;
        for(size_t i=0; i < cvar_name.size(); ++i) {
            if(cvar_name[i] == ' ') cvar_name[i] = '_';
        }
        
        // Attach to CVars too.
        const char* typeidname = var.type_name;
        if( !strcmp(typeidname, typeid(double).name()) ) {
            CVarUtils::AttachCVar(cvar_name, (double*)(var.val) );
        } else if( !strcmp(typeidname, typeid(int).name()) ) {
            CVarUtils::AttachCVar(cvar_name, (int*)(var.val) );
        } else if( !strcmp(typeidname, typeid(std::string).name()) ) {
            CVarUtils::AttachCVar(cvar_name, (std::string*)(var.val) );
        } else if( !strcmp(typeidname, typeid(bool).name()) ) {
            CVarUtils::AttachCVar(cvar_name, (bool*)(var.val) );
        } else {
            // we can't attach
            pango_print_error("NewVarForCVars: Typeid '%s' does not match known types.\n", typeidname);
        }
    }
}
#endif // BUILD_PANGOLIN_VARS
#endif // HAVE_CVARS

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
#ifdef HAVE_GLCONSOLE
    else if(key == '`') {
        context->console.ToggleConsole();
        // Force refresh for several frames whilst panel opens/closes
        context->had_input = 60*2;
    }else if(context->console.IsOpen()) {
        // Direct input to console
        if( key >= 128 ) {
            context->console.SpecialFunc(key - 128 );
        }else{
            context->console.KeyboardFunc(key);
        }
    }
#endif // HAVE_GLCONSOLE
#ifdef HAVE_GLUT
    else if( key == GLUT_KEY_TAB) {
        if( context->is_fullscreen )
        {
            glutReshapeWindow(context->windowed_size[0],context->windowed_size[1]);
            context->is_fullscreen = false;
        }else{
            glutFullScreen();
            context->is_fullscreen = true;
        }
    }
#endif // HAVE_GLUT
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
    
    last_x = x;
    last_y = y;
    
    const MouseButton button = (MouseButton)(1 << button_raw);
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
    
    last_x = x;
    last_y = y;
    
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
    
    last_x = x;
    last_y = y;
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
#ifdef HAVE_CVARS
    
#ifdef BUILD_PANGOLIN_VARS
    RegisterNewVarCallback(NewVarForCVars,0);
#endif // BUILD_PANGOLIN_VARS
    
    // Register utilities
    CVarUtils::CreateCVar("pango.view.list",  &CVarViewList, "List named views." );
    CVarUtils::CreateCVar("pango.view.showhide",  &CVarViewShowHide, "Show/Hide named view." );
    CVarUtils::CreateCVar("pango.screencap", &CVarScreencap, "Capture image of window to a file." );
#ifdef BUILD_PANGOLIN_VIDEO
    CVarUtils::CreateCVar("pango.record.start", &CVarRecordStart, "Record video of window to a file." );
    CVarUtils::CreateCVar("pango.record.stop",  &CVarRecordStop, "Stop video recording." );
#endif // BUILD_PANGOLIN_VIDEO

#endif // HAVE_CVARS
}


}
