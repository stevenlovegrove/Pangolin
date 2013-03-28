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
#include <pangolin/gl.h>
#include <pangolin/display.h>
#include <pangolin/display_internal.h>
#include <pangolin/simple_math.h>

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

namespace pangolin
{

typedef boost::ptr_unordered_map<std::string,PangolinGl> ContextMap;

// Map of active contexts
ContextMap contexts;

// Context active for current thread
__thread PangolinGl* context = 0;

PangolinGl::PangolinGl()
    : quit(false), mouse_state(0), activeDisplay(0)
{
}

void BindToContext(std::string name)
{
    ContextMap::iterator ic = contexts.find(name);
    
    if( ic == contexts.end() )
    {
        // Create and add if not found
        ic = contexts.insert( name,new PangolinGl ).first;
        context = ic->second;
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
        context = ic->second;
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
    boost::ptr_unordered_map<std::string,View>::iterator vi = context->named_managed_views.find(name);
    if( vi != context->named_managed_views.end() )
    {
        return *(vi->second);
    }else{
        View * v = new View();
        bool inserted = context->named_managed_views.insert(name, v).second;
        if(!inserted) throw std::exception();
        v->handler = &StaticHandler;
        context->base.views.push_back(v);
        return *v;
    }
}

void RegisterKeyPressCallback(int key, boost::function<void(void)> func)
{
    context->keypress_hooks[key] = func;
}

void SaveWindowOnRender(std::string prefix)
{
    context->screen_capture.push(std::pair<std::string,Viewport>(prefix, context->base.v) );
}

namespace process
{
unsigned int last_x;
unsigned int last_y;

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
#ifdef HAVE_CVARS
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
#endif // HAVE_CVARS
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

void SubpixTabletMotion(float x, float y, float pressure, float rotation, float tiltx, float tilty)
{
    SpecialInput(InputSpecialTablet, x, y, pressure, rotation, tiltx, tilty);
}
}

#ifdef HAVE_GLUT
void PangoGlutRedisplay()
{
    glutPostRedisplay();
    
    //      RenderViews();
    //      FinishGlutFrame();
}

void TakeGlutCallbacks()
{
    glutKeyboardFunc(&process::Keyboard);
    glutKeyboardUpFunc(&process::KeyboardUp);
    glutReshapeFunc(&process::Resize);
    glutMouseFunc(&process::Mouse);
    glutMotionFunc(&process::MouseMotion);
    glutPassiveMotionFunc(&process::PassiveMouseMotion);
    glutSpecialFunc(&process::SpecialFunc);
    glutSpecialUpFunc(&process::SpecialFuncUp);
    
#ifdef HAVE_APPLE_OPENGL_FRAMEWORK
    glutDisplayFunc(&PangoGlutRedisplay);
    
    // Attempt to register special smooth scroll callback
    // https://github.com/nanoant/osxglut
    typedef void (*glutScrollFunc_t)(void (*)(float, float));
    typedef void (*glutZoomFunc_t)(void (*)(float));
    typedef void (*glutRotateFunc_t)(void (*)(float));
    typedef void (*glutSubpixTabletMotionFunc_t)(void (*)(float,float,float,float,float,float));
    
    glutScrollFunc_t glutScrollFunc = (glutScrollFunc_t)glutGetProcAddress("glutScrollFunc");
    glutZoomFunc_t glutZoomFunc = (glutZoomFunc_t)glutGetProcAddress("glutZoomFunc");
    glutRotateFunc_t glutRotateFunc = (glutRotateFunc_t)glutGetProcAddress("glutRotateFunc");
    glutSubpixTabletMotionFunc_t glutSubpixTabletMotionFunc = (glutSubpixTabletMotionFunc_t)glutGetProcAddress("glutSubpixTabletMotionFunc");
    
    if(glutScrollFunc) {
        glutScrollFunc(&process::Scroll);
    }
    if(glutZoomFunc) {
        glutZoomFunc(&process::Zoom);
    }
    if(glutRotateFunc) {
        glutRotateFunc(&process::Rotate);
    }
    
    if(glutSubpixTabletMotionFunc) {
        glutSubpixTabletMotionFunc(&process::SubpixTabletMotion);
    }
    
#endif
}


void SaveFramebuffer(std::string prefix, const Viewport& v)
{
#ifdef HAVE_BOOST_GIL
    // Save colour channels
    boost::gil::rgba8_image_t img(v.w, v.h);
    glReadPixels(v.l, v.b, v.w, v.h, GL_RGBA, GL_UNSIGNED_BYTE, boost::gil::interleaved_view_get_raw_data( boost::gil::view( img ) ) );
#ifdef HAVE_PNG
    boost::gil::png_write_view(prefix + ".png", flipped_up_down_view( boost::gil::const_view(img)) );
#endif // HAVE_PNG
    
    //      // Save depth channel
    //      boost::gil::gray32f_image_t depth(v.w, v.h);
    //      glReadPixels(v.l, v.b, v.w, v.h, GL_DEPTH_COMPONENT, GL_FLOAT, boost::gil::interleaved_view_get_raw_data( view( depth ) ));
    //      boost::gil::tiff_write_view(prefix + "_depth.tiff", flipped_up_down_view(const_view(depth)) );
#endif // HAVE_BOOST_GIL
}

#ifdef BUILD_PANGOLIN_VARS
#ifdef HAVE_CVARS
void NewVarForCVars(void* /*data*/, const std::string& name, _Var& var, const char* /*orig_typeidname*/, bool brand_new)
{
    if(brand_new) {
        // Attach to CVars too.
        const char* typeidname = var.type_name;
        if( typeidname == typeid(double).name() ) {
            CVarUtils::AttachCVar(name, (double*)(var.val) );
        } else if( typeidname == typeid(int).name() ) {
            CVarUtils::AttachCVar(name, (int*)(var.val) );
        } else if( typeidname == typeid(std::string).name() ) {
            CVarUtils::AttachCVar(name, (std::string*)(var.val) );
        } else if( typeidname == typeid(bool).name() ) {
            CVarUtils::AttachCVar(name, (bool*)(var.val) );
        } else {
            // we can't attach
            std::cerr << typeidname << std::endl;
        }
    }
}
#endif // HAVE_CVARS
#endif // BUILD_PANGOLIN_VARS

void CreateGlutWindowAndBind(std::string window_title, int w, int h, unsigned int mode)
{
#ifdef HAVE_FREEGLUT
    if( glutGet(GLUT_INIT_STATE) == 0)
#endif
    {
        int argc = 0;
        glutInit(&argc, 0);
        glutInitDisplayMode(mode);
    }
    glutInitWindowSize(w,h);
    glutCreateWindow(window_title.c_str());
    BindToContext(window_title);
    
#ifdef HAVE_FREEGLUT
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
#endif
    
    context->is_double_buffered = mode & GLUT_DOUBLE;
    TakeGlutCallbacks();
    
#ifdef BUILD_PANGOLIN_VARS
#ifdef HAVE_CVARS
    RegisterNewVarCallback(NewVarForCVars,0);
#endif // HAVE_CVARS
#endif // BUILD_PANGOLIN_VARS
}

void FinishGlutFrame()
{
    RenderViews();
    DisplayBase().Activate();
    Viewport::DisableScissor();
    
#ifdef HAVE_BOOST_GIL
    while(context->screen_capture.size()) {
        std::pair<std::string,Viewport> fv = context->screen_capture.front();
        context->screen_capture.pop();
        SaveFramebuffer(fv.first, fv.second);
    }
#endif // HAVE_BOOST_GIL
    
#ifdef HAVE_CVARS
    context->console.RenderConsole();
#endif // HAVE_CVARS
    SwapGlutBuffersProcessGlutEvents();
}

void SwapGlutBuffersProcessGlutEvents()
{
    glutSwapBuffers();
    
#ifdef HAVE_FREEGLUT
    glutMainLoopEvent();
#endif
    
#ifdef HAVE_GLUT_APPLE_FRAMEWORK
    glutCheckLoop();
#endif
}
#endif // HAVE_GLUT

void DrawTextureToViewport(GLuint texid)
{
    OpenGlRenderState::ApplyIdentity();
    glBindTexture(GL_TEXTURE_2D, texid);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2d(-1,-1);
    glTexCoord2f(1, 0);
    glVertex2d(1,-1);
    glTexCoord2f(1, 1);
    glVertex2d(1,1);
    glTexCoord2f(0, 1);
    glVertex2d(-1,1);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

}
