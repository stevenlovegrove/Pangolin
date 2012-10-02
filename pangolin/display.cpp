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
#include <map>

#include <boost/function.hpp>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "platform.h"
#include "gl.h"
#include "display.h"
#include "display_internal.h"
#include "simple_math.h"

#ifdef BUILD_PANGOLIN_VARS
  #include "vars.h"
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

using namespace std;

namespace pangolin
{

  const int panal_v_margin = 6;

  typedef boost::ptr_unordered_map<string,PangolinGl> ContextMap;

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
      if(!inserted) throw exception();
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

    void SpecialInput(InputSpecial inType, int x, int y, float p1, float p2, float p3, float p4)
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

    glutScrollFunc_t glutScrollFunc = (glutScrollFunc_t)glutGetProcAddress("glutScrollFunc");
    glutZoomFunc_t glutZoomFunc = (glutZoomFunc_t)glutGetProcAddress("glutZoomFunc");
    glutRotateFunc_t glutRotateFunc = (glutRotateFunc_t)glutGetProcAddress("glutRotateFunc");

    if(glutScrollFunc) {
        glutScrollFunc(&process::Scroll);
    }
    if(glutZoomFunc) {
        glutZoomFunc(&process::Zoom);
    }
    if(glutRotateFunc) {
        glutRotateFunc(&process::Rotate);
    }

#endif
  }


  void SaveViewFromFbo(std::string prefix, View& view, float scale)
  {
      const Viewport orig = view.v;
      view.v.l = 0;
      view.v.b = 0;
      view.v.w *= scale;
      view.v.h *= scale;

      const int w = view.v.w;
      const int h = view.v.h;

      float origLineWidth;
      glGetFloatv(GL_LINE_WIDTH, &origLineWidth);
      glLineWidth(origLineWidth * scale);

      // Create FBO
      GlTexture color(w,h);
      GlRenderBuffer depth(w,h);
      GlFramebuffer fbo(color, depth);

      // Render into FBO
      fbo.Bind();
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      view.Render();
      glFlush();

#ifdef HAVE_BOOST_GIL
      // Read buffer and write file
      boost::gil::rgba8_image_t img(w, h);
      glReadPixels(0,0,w,h, GL_RGBA, GL_UNSIGNED_BYTE, boost::gil::interleaved_view_get_raw_data( boost::gil::view( img ) ) );
#ifdef HAVE_PNG
      boost::gil::png_write_view(prefix + ".png", flipped_up_down_view( boost::gil::const_view(img)) );
#endif // HAVE_PNG
#endif // HAVE_BOOST_GIL

      // unbind FBO
      fbo.Unbind();

      // restore viewport / line width
      view.v = orig;
      glLineWidth(origLineWidth);
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

  void CreateGlutWindowAndBind(string window_title, int w, int h, unsigned int mode)
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

  void Viewport::Activate() const
  {
    glViewport(l,b,w,h);
  }

  void Viewport::Scissor() const
  {
    glEnable(GL_SCISSOR_TEST);
    glScissor(l,b,w,h);
  }

  void Viewport::ActivateAndScissor() const
  {
    glViewport(l,b,w,h);
    glEnable(GL_SCISSOR_TEST);
    glScissor(l,b,w,h);
  }


  void Viewport::DisableScissor()
  {
    glDisable(GL_SCISSOR_TEST);
  }

  bool Viewport::Contains(int x, int y) const
  {
    return l <= x && x < (l+w) && b <= y && y < (b+h);
  }

  Viewport Viewport::Inset(int i) const
  {
    return Viewport(l+i, b+i, w-2*i, h-2*i);
  }

  Viewport Viewport::Inset(int horiz, int vert) const
  {
    return Viewport(l+horiz, b+vert, w-horiz, h-vert);
  }

  Viewport Viewport::Intersect(const Viewport& vp) const
  {
      GLint nl = std::max(l,vp.l);
      GLint nr = std::min(r(),vp.r());
      GLint nb = std::max(b,vp.b);
      GLint nt = std::min(t(),vp.t());
      return Viewport(nl,nb, nr-nl, nt-nb);
  }

  void OpenGlMatrix::Load() const
  {
    glLoadMatrixd(m);
  }

  void OpenGlMatrix::Multiply() const
  {
    glMultMatrixd(m);
  }

  void OpenGlMatrix::SetIdentity()
  {
      m[0] = 1.0f;  m[1] = 0.0f;  m[2] = 0.0f;  m[3] = 0.0f;
      m[4] = 0.0f;  m[5] = 1.0f;  m[6] = 0.0f;  m[7] = 0.0f;
      m[8] = 0.0f;  m[9] = 0.0f; m[10] = 1.0f; m[11] = 0.0f;
     m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;
  }

  OpenGlMatrix OpenGlMatrix::Inverse() const
  {
      OpenGlMatrix inv;
      inv.m[0] = m[0]; inv.m[4] = m[1]; inv.m[8]  = m[2];  inv.m[12] = -(m[0]*m[12] + m[1]*m[13] + m[2]*m[14]);
      inv.m[1] = m[4]; inv.m[5] = m[5]; inv.m[9]  = m[6];  inv.m[13] = -(m[4]*m[12] + m[5]*m[13] + m[6]*m[14]);
      inv.m[2] = m[8]; inv.m[6] = m[9]; inv.m[10] = m[10]; inv.m[14] = -(m[8]*m[12] + m[9]*m[13] + m[10]*m[14]);
      inv.m[3] =    0; inv.m[7] =    0; inv.m[11] =    0;  inv.m[15] = 1;
      return inv;
  }

  void OpenGlRenderState::Apply() const
  {
    // Apply any stack matrices we have
    for(map<OpenGlStack,OpenGlMatrix>::const_iterator i = stacks.begin(); i != stacks.end(); ++i )
    {
      glMatrixMode(i->first);
      i->second.Load();
    }

    // Leave in MODEVIEW mode
    glMatrixMode(GL_MODELVIEW);

    if(follow) {
        T_cw.Multiply();
    }
  }

  OpenGlRenderState::OpenGlRenderState()
      : follow(false)
  {
  }

  OpenGlRenderState::OpenGlRenderState(const OpenGlMatrix& projection_matrix)
      : follow(false)
  {
    stacks[GlProjectionStack] = projection_matrix;
    stacks[GlModelViewStack] = IdentityMatrix();
  }

  OpenGlRenderState::OpenGlRenderState(const OpenGlMatrix& projection_matrix, const OpenGlMatrix& modelview_matrx)
      : follow(false)
  {
    stacks[GlProjectionStack] = projection_matrix;
    stacks[GlModelViewStack] = modelview_matrx;
  }

  void OpenGlRenderState::ApplyIdentity()
  {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
  }

  void OpenGlRenderState::ApplyWindowCoords()
  {
    context->base.v.Activate();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, context->base.v.w, 0, context->base.v.h);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
  }

  OpenGlRenderState& OpenGlRenderState::SetProjectionMatrix(OpenGlMatrix spec)
  {
      stacks[GlProjectionStack] = spec;
      return *this;
  }

  OpenGlRenderState& OpenGlRenderState::SetModelViewMatrix(OpenGlMatrix spec)
  {
      stacks[GlModelViewStack] = spec;
      return *this;
  }

  OpenGlRenderState& OpenGlRenderState::Set(OpenGlMatrixSpec spec)
  {
    stacks[spec.type] = spec;
    return *this;
  }

  OpenGlMatrix operator*(const OpenGlMatrix& lhs, const OpenGlMatrix& rhs)
  {
      OpenGlMatrix ret;
      pangolin::MatMul<4,4,4,double>(ret.m, lhs.m, rhs.m);
      return ret;
  }

  OpenGlMatrix& OpenGlRenderState::GetProjectionMatrix()
  {
      return stacks[GlProjectionStack];
  }

  OpenGlMatrix OpenGlRenderState::GetProjectionMatrix() const
  {
      std::map<OpenGlStack,OpenGlMatrix>::const_iterator i = stacks.find(GlProjectionStack);
      if( i == stacks.end() ) {
        return IdentityMatrix();
      }else{
        return i->second;
      }
  }

  OpenGlMatrix& OpenGlRenderState::GetModelViewMatrix()
  {
      return stacks[GlModelViewStack];
  }

  OpenGlMatrix OpenGlRenderState::GetModelViewMatrix() const
  {
      std::map<OpenGlStack,OpenGlMatrix>::const_iterator i = stacks.find(GlModelViewStack);
      if( i == stacks.end() ) {
        return IdentityMatrix();
      }else{
        return i->second;
      }
  }

  void OpenGlRenderState::Follow(const OpenGlMatrix& T_wc, bool follow)
  {
      this->T_cw = T_wc.Inverse();

      if(follow != this->follow) {
          if(follow) {
              const OpenGlMatrix T_vc = GetModelViewMatrix() * T_wc;
              SetModelViewMatrix(T_vc);
              this->follow = true;
          }else{
              Unfollow();
          }
      }
  }

  void OpenGlRenderState::Unfollow()
  {
      const OpenGlMatrix T_vw = GetModelViewMatrix() * T_cw;
      SetModelViewMatrix(T_vw);
      this->follow = false;
  }

  int AttachAbs( int low, int high, Attach a)
  {
    if( a.unit == Pixel ) return low + a.p;
    if( a.unit == ReversePixel ) return high - a.p;
    return low + a.p * (high - low);
  }

  float AspectAreaWithinTarget(double target, double test)
  {
    if( test < target )
      return test / target;
    else
      return target / test;
  }

  void View::Resize(const Viewport& p)
  {
    // Compute Bounds based on specification
    v.l = AttachAbs(p.l,p.r(),left);
    v.b = AttachAbs(p.b,p.t(),bottom);
    int r = AttachAbs(p.l,p.r(),right);
    int t = AttachAbs(p.b,p.t(),top);

    // Make sure left and right, top and bottom are correct order
    if( t < v.b ) swap(t,v.b);
    if( r < v.l ) swap(r,v.l);

    v.w = r - v.l;
    v.h = t - v.b;

    vp = v;

    // Adjust based on aspect requirements
    if( aspect != 0 )
    {
      const float current_aspect = (float)v.w / (float)v.h;
      if( aspect > 0 )
      {
        // Fit to space
        if( current_aspect < aspect )
        {
          //Adjust height
          const int nh = (int)(v.w / aspect);
          v.b += vlock == LockBottom ? 0 : (vlock == LockCenter ? (v.h-nh)/2 : (v.h-nh) );
          v.h = nh;
        }else if( current_aspect > aspect )
        {
          //Adjust width
          const int nw = (int)(v.h * aspect);
          v.l += hlock == LockLeft? 0 : (hlock == LockCenter ? (v.w-nw)/2 : (v.w-nw) );
          v.w = nw;
        }
      }else{
        // Overfit
        double true_aspect = -aspect;
        if( current_aspect < true_aspect )
        {
          //Adjust width
          const int nw = (int)(v.h * true_aspect);
          v.l += hlock == LockLeft? 0 : (hlock == LockCenter ? (v.w-nw)/2 : (v.w-nw) );
          v.w = nw;
        }else if( current_aspect > true_aspect )
        {
          //Adjust height
          const int nh = (int)(v.w / true_aspect);
          v.b += vlock == LockBottom ? 0 : (vlock == LockCenter ? (v.h-nh)/2 : (v.h-nh) );
          v.h = nh;
        }
      }
    }

    ResizeChildren();
  }

  void View::ResizeChildren()
  {
    if( layout == LayoutOverlay )
    {
      foreach(View* i, views)
        i->Resize(v);
    }else if( layout == LayoutVertical )
    {
      // Allocate space incrementally
      Viewport space = v.Inset(panal_v_margin);
      int num_children = 0;
      foreach(View* i, views )
      {
        num_children++;
        if(scroll_offset > num_children ) {
            i->show = false;
        }else{
            i->show = true;
            i->Resize(space);
            space.h = i->v.b - panal_v_margin - space.b;
        }
      }
    }else if(layout == LayoutHorizontal )
    {
      // Allocate space incrementally
      const int margin = 8;
      Viewport space = v.Inset(margin);
      foreach(View* i, views )
      {
        i->Resize(space);
        space.w = i->v.l + margin + space.l;
      }
    }else if(layout == LayoutEqual )
    {
      const size_t visiblechildren = NumVisibleChildren();
      // TODO: Make this neater, and make fewer assumptions!
      if( visiblechildren > 0 )
      {
        // This containers aspect
        const double this_a = abs(v.aspect());

        // Use first child with fixed aspect for all children
        double child_a = abs(VisibleChild(0).aspect);
        for(size_t i=1; (child_a==0) && i < visiblechildren; ++i ) {
            child_a = abs(VisibleChild(i).aspect);
        }

        if(child_a == 0) {
            std::cerr << "LayoutEqual requires that each child has same aspect, but no child with fixed aspect found. Using 1:1." << std::endl;
            child_a = 1;
        }

        double a = visiblechildren*child_a;
        double area = AspectAreaWithinTarget(this_a, a);

        int cols = visiblechildren-1;
        for(; cols > 0; --cols)
        {
          const int rows = visiblechildren / cols + (visiblechildren % cols == 0 ? 0 : 1);
          const double na = cols * child_a / rows;
          const double new_area = visiblechildren*AspectAreaWithinTarget(this_a,na)/(rows*cols);
          if( new_area <= area )
            break;
          area = new_area;
          a = na;
        }

        cols++;
        const int rows = visiblechildren / cols + (visiblechildren % cols == 0 ? 0 : 1);
        int cw,ch;
        if( a > this_a )
        {
          cw = v.w / cols;
          ch = cw / child_a; //v.h / rows;
        }else{
          ch = v.h / rows;
          cw = ch * child_a;
        }

        for( unsigned int i=0; i< visiblechildren; ++i )
        {
          int c = i % cols;
          int r = i / cols;
          Viewport space(v.l + c*cw, v.t() - (r+1)*ch, cw,ch);
          VisibleChild(i).Resize(space);
        }
      }
    }

  }

  void View::Render()
  {
    if(!extern_draw_function.empty() && show) {
      extern_draw_function(*this);
    }
    RenderChildren();
  }

  void View::RenderChildren()
  {
    foreach(View* v, views)
      if(v->show) v->Render();
  }

  void View::Activate() const
  {
    v.Activate();
  }

  void View::ActivateAndScissor() const
  {
    vp.Scissor();
    v.Activate();
  }

  void View::ActivateScissorAndClear() const
  {
    vp.Scissor();
    v.Activate();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void View::Activate(const OpenGlRenderState& state ) const
  {
    v.Activate();
    state.Apply();
  }

  void View::ActivateAndScissor(const OpenGlRenderState& state) const
  {
    vp.Scissor();
    v.Activate();
    state.Apply();
  }

  void View::ActivateScissorAndClear(const OpenGlRenderState& state ) const
  {
    vp.Scissor();
    v.Activate();
    state.Apply();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  GLfloat View::GetClosestDepth(int x, int y, int radius) const
  {
      glReadBuffer(GL_FRONT);
      const int zl = (radius*2+1);
      const int zsize = zl*zl;
      GLfloat zs[zsize];
      glReadPixels(x-radius,y-radius,zl,zl,GL_DEPTH_COMPONENT,GL_FLOAT,zs);
      const GLfloat mindepth = *(std::min_element(zs,zs+zsize));
      return mindepth;
  }

  void View::GetObjectCoordinates(const OpenGlRenderState& cam_state, double winx, double winy, double winzdepth, double& x, double& y, double& z) const
  {
      const GLint viewport[4] = {v.l,v.b,v.w,v.h};
      const OpenGlMatrix proj = cam_state.GetProjectionMatrix();
      const OpenGlMatrix mv = cam_state.GetModelViewMatrix();
      gluUnProject(winx, winy, winzdepth, mv.m, proj.m, viewport, &x, &y, &z);
  }

  void View::GetCamCoordinates(const OpenGlRenderState& cam_state, double winx, double winy, double winzdepth, double& x, double& y, double& z) const
  {
      const GLint viewport[4] = {v.l,v.b,v.w,v.h};
      const OpenGlMatrix proj = cam_state.GetProjectionMatrix();
      gluUnProject(winx, winy, winzdepth, Identity4d, proj.m, viewport, &x, &y, &z);
  }

  View& View::SetFocus()
  {
    context->activeDisplay = this;
    return *this;
  }

  View& View::SetBounds(Attach bottom, Attach top,  Attach left, Attach right, bool keep_aspect)
  {
    SetBounds(top,bottom,left,right,0.0);
    aspect = keep_aspect ? v.aspect() : 0;
    return *this;
  }

  View& View::SetBounds(Attach bottom, Attach top,  Attach left, Attach right, double aspect)
  {
    this->left = left;
    this->top = top;
    this->right = right;
    this->bottom = bottom;
    this->aspect = aspect;
    context->base.ResizeChildren();
    return *this;
  }

  View& View::SetAspect(double aspect)
  {
    this->aspect = aspect;
    context->base.ResizeChildren();
    return *this;
  }

  View& View::SetLock(Lock horizontal, Lock vertical )
  {
    vlock = vertical;
    hlock = horizontal;
    return *this;
  }

  View& View::SetLayout(Layout l)
  {
    layout = l;
    return *this;
  }


  View& View::AddDisplay(View& child)
  {
    // detach child from any other view, and add to this
    vector<View*>::iterator f = std::find(
      context->base.views.begin(), context->base.views.end(), &child
    );

    if( f != context->base.views.end() )
      context->base.views.erase(f);

    views.push_back(&child);
    context->base.ResizeChildren();
    return *this;
  }

  View& View::Show(bool show)
  {
      this->show = show;
      context->base.ResizeChildren();
      return *this;
  }

  void View::ToggleShow()
  {
      Show(!show);
  }

  bool View::IsShown() const
  {
      return show;
  }

  void View::SaveOnRender(const std::string& filename_prefix)
  {
      const Viewport tosave = this->v.Intersect(this->vp);
      context->screen_capture.push(std::pair<std::string,Viewport>(filename_prefix,tosave ) );
  }

  void View::SaveRenderNow(const std::string& filename_prefix, float scale)
  {
      SaveViewFromFbo(filename_prefix, *this, scale);
  }

  View& View::operator[](size_t i)
  {
      return *views[i];
  }

  size_t View::NumChildren() const
  {
      return views.size();
  }

  size_t View::NumVisibleChildren() const
  {
      int numvis = 0;
      for(std::vector<View*>::const_iterator i=views.begin(); i!=views.end(); ++i)
      {
          if((*i)->show) {
              numvis++;
          }
      }
      return numvis;
  }

  View& View::VisibleChild(size_t i)
  {
      size_t numvis = 0;
      for(size_t v=0; v < views.size(); ++v ) {
          if(views[v]->show) {
              if( i == numvis ) {
                  return *views[v];
              }
              numvis++;
          }
      }
      // Shouldn't get here
      assert(0);
      return *this;
  }

  View& View::SetHandler(Handler* h)
  {
    handler = h;
    return *this;
  }

  View& View::SetDrawFunction(const boost::function<void(View&)>& drawFunc)
  {
    extern_draw_function = drawFunc;
    return *this;
  }

  View* FindChild(View& parent, int x, int y)
  {
    // Find in reverse order to mirror draw order
    for( vector<View*>::const_reverse_iterator i = parent.views.rbegin(); i != parent.views.rend(); ++i )
      if( (*i)->show && (*i)->vp.Contains(x,y) )
        return (*i);
    return 0;
  }

  void Handler::Keyboard(View& d, unsigned char key, int x, int y, bool pressed)
  {
    View* child = FindChild(d,x,y);
    if( child)
    {
      context->activeDisplay = child;
      if( child->handler)
        child->handler->Keyboard(*child,key,x,y,pressed);
    }
  }

  void Handler::Mouse(View& d, MouseButton button, int x, int y, bool pressed, int button_state)
  {
    View* child = FindChild(d,x,y);
    if( child )
    {
      context->activeDisplay = child;
      if( child->handler)
        child->handler->Mouse(*child,button,x,y,pressed,button_state);
    }
  }

  void Handler::MouseMotion(View& d, int x, int y, int button_state)
  {
    View* child = FindChild(d,x,y);
    if( child )
    {
      context->activeDisplay = child;
      if( child->handler)
        child->handler->MouseMotion(*child,x,y,button_state);
    }
  }

  void Handler::Special(View& d, InputSpecial inType,  int x, int y, float p1, float p2, float p3, float p4, int button_state)
  {
      View* child = FindChild(d,x,y);
      if( child )
      {
        context->activeDisplay = child;
        if( child->handler)
          child->handler->Special(*child,inType, x,y, p1, p2, p3, p4, button_state);
      }
  }

  void HandlerScroll::Mouse(View& d, MouseButton button, int x, int y, bool pressed, int button_state)
  {
    if( button == button_state && (button == MouseWheelUp || button == MouseWheelDown) )
    {
        if( button == MouseWheelUp) d.scroll_offset   -= 1;
        if( button == MouseWheelDown) d.scroll_offset += 1;
        d.scroll_offset = max(0, min(d.scroll_offset, (int)d.views.size()) );
        d.ResizeChildren();
    }else{
        Handler::Mouse(d,button,x,y,pressed,button_state);
    }
  }

  void HandlerScroll::Special(View& d, InputSpecial inType, int x, int y, float p1, float p2, float p3, float p4, int button_state)
  {
    if( inType == InputSpecialScroll )
    {
        d.scroll_offset -= p2 / abs(p2);
        d.scroll_offset = max(0, min(d.scroll_offset, (int)d.views.size()) );
        d.ResizeChildren();
    }else{
        Handler::Special(d,inType,x,y,p1,p2,p3,p4,button_state);
    }
  }


  void Handler3D::Keyboard(View&, unsigned char key, int x, int y, bool pressed)
  {
    // TODO: hooks for reset / changing mode (perspective / ortho etc)
  }

  void Handler3D::Mouse(View& display, MouseButton button, int x, int y, bool pressed, int button_state)
  {
    // mouse down
    last_pos[0] = x;
    last_pos[1] = y;

    double T_nc[3*4];
    LieSetIdentity(T_nc);

    if( pressed ) {
      const GLfloat mindepth = display.GetClosestDepth(x,y,hwin);
      last_z = mindepth != 1 ? mindepth : last_z;

      if( last_z != 1 ) {
        display.GetCamCoordinates(*cam_state, x, y, last_z, rot_center[0], rot_center[1], rot_center[2]);
      }else{
        SetZero<3,1>(rot_center);
      }

      if( button == MouseWheelUp || button == MouseWheelDown)
      {
        LieSetIdentity(T_nc);
        const double t[] = { 0,0,(button==MouseWheelUp?1:-1)*100*tf};
        LieSetTranslation<>(T_nc,t);
        if( !(button_state & MouseButtonRight) && !(rot_center[0]==0 && rot_center[1]==0 && rot_center[2]==0) )
        {
          LieSetTranslation<>(T_nc,rot_center);
          MatMul<3,1>(T_nc+(3*3),(button==MouseWheelUp?-1.0:1.0) * zf);
        }
        OpenGlMatrix& spec = cam_state->GetModelViewMatrix();
        LieMul4x4bySE3<>(spec.m,T_nc,spec.m);
      }
    }
  }

  // Direction vector for each AxisDirection enum
  const static GLdouble AxisDirectionVector[][3] = {
      {0,0,0},
      {-1,0,0}, {1,0,0},
      {0,-1,0}, {0,1,0},
      {0,0,-1}, {0,0,1}
  };

  void Handler3D::MouseMotion(View& display, int x, int y, int button_state)
  {
    const int delta[2] = {(x-last_pos[0]),(y-last_pos[1])};
    const float mag = delta[0]*delta[0] + delta[1]*delta[1];

    // TODO: convert delta to degrees based of fov
    // TODO: make transformation with respect to cam spec

    if( mag < 50*50 )
    {
      OpenGlMatrix& mv = cam_state->GetModelViewMatrix();
      const GLdouble* up = AxisDirectionVector[enforce_up];
      double T_nc[3*4];
      LieSetIdentity(T_nc);
      bool rotation_changed = false;

      if( button_state == MouseButtonMiddle )
      {
        // Middle Drag: in plane translate
        Rotation<>(T_nc,-delta[1]*0.01, -delta[0]*0.01, 0.0);
      }else if( button_state == MouseButtonLeft )
      {
        // Left Drag: in plane translate
        if( last_z != 1 )
        {
          GLdouble np[3];
          display.GetCamCoordinates(*cam_state,x,y,last_z, np[0], np[1], np[2]);
          const double t[] = { np[0] - rot_center[0], np[1] - rot_center[1], 0};
          LieSetTranslation<>(T_nc,t);
          std::copy(np,np+3,rot_center);
        }else{
          const double t[] = { -10*delta[0]*tf, 10*delta[1]*tf, 0};
          LieSetTranslation<>(T_nc,t);
        }
      }else if( button_state == (MouseButtonLeft | MouseButtonRight) )
      {
        // Left and Right Drag: in plane rotate about object
//        Rotation<>(T_nc,0.0,0.0, delta[0]*0.01);

        double T_2c[3*4];
        Rotation<>(T_2c,0.0,0.0, delta[0]*0.01);
        double mrotc[3];
        MatMul<3,1>(mrotc, rot_center, -1.0);
        LieApplySO3<>(T_2c+(3*3),T_2c,mrotc);
        double T_n2[3*4];
        LieSetIdentity<>(T_n2);
        LieSetTranslation<>(T_n2,rot_center);
        LieMulSE3(T_nc, T_n2, T_2c );
        rotation_changed = true;
      }else if( button_state == MouseButtonRight)
      {
        // Correct for OpenGL Camera.
        double aboutx = -0.01 * delta[1];
        double abouty =  0.01 * delta[0];

        // Try to correct for different coordinate conventions.
        OpenGlMatrix& pm = cam_state->GetProjectionMatrix();
        abouty *= -pm.m[2*4+3];

        if(enforce_up) {
            // Special case if view direction is parallel to up vector
            const double updotz = mv.m[2]*up[0] + mv.m[6]*up[1] + mv.m[10]*up[2];
            if(updotz > 0.98) aboutx = std::min(aboutx,0.0);
            if(updotz <-0.98) aboutx = std::max(aboutx,0.0);
            // Module rotation around y so we don't spin too fast!
            abouty *= (1-0.6*abs(updotz));
        }

        // Right Drag: object centric rotation
        double T_2c[3*4];
        Rotation<>(T_2c, aboutx, abouty, 0.0);
        double mrotc[3];
        MatMul<3,1>(mrotc, rot_center, -1.0);
        LieApplySO3<>(T_2c+(3*3),T_2c,mrotc);
        double T_n2[3*4];
        LieSetIdentity<>(T_n2);
        LieSetTranslation<>(T_n2,rot_center);
        LieMulSE3(T_nc, T_n2, T_2c );
        rotation_changed = true;
      }

      LieMul4x4bySE3<>(mv.m,T_nc,mv.m);

      if(enforce_up != AxisNone && rotation_changed) {
          EnforceUpT_cw(mv.m, up);
      }
    }

    last_pos[0] = x;
    last_pos[1] = y;
  }

  void Handler3D::Special(View& display, InputSpecial inType, int x, int y, float p1, float p2, float p3, float p4, int button_state)
  {
    // mouse down
    last_pos[0] = x;
    last_pos[1] = y;

    double T_nc[3*4];
    LieSetIdentity(T_nc);

    const GLfloat mindepth = display.GetClosestDepth(x,y,hwin);
    last_z = mindepth != 1 ? mindepth : last_z;

    if( last_z != 1 ) {
        display.GetCamCoordinates(*cam_state, last_pos[0], last_pos[1], last_z, rot_center[0], rot_center[1], rot_center[2]);
    }else{
      SetZero<3,1>(rot_center);
    }

    if( inType == InputSpecialScroll ) {
      const double scrolly = p2/10;

      LieSetIdentity(T_nc);
      const double t[] = { 0,0, -scrolly*100*tf};
      LieSetTranslation<>(T_nc,t);
      if( !(button_state & MouseButtonRight) && !(rot_center[0]==0 && rot_center[1]==0 && rot_center[2]==0) )
      {
        LieSetTranslation<>(T_nc,rot_center);
        MatMul<3,1>(T_nc+(3*3), -scrolly * zf);
      }
      OpenGlMatrix& spec = cam_state->GetModelViewMatrix();
      LieMul4x4bySE3<>(spec.m,T_nc,spec.m);
    }else if(inType == InputSpecialRotate) {
        const double r = p1 / 20;

        double T_2c[3*4];
        Rotation<>(T_2c,0.0,0.0, r);
        double mrotc[3];
        MatMul<3,1>(mrotc, rot_center, -1.0);
        LieApplySO3<>(T_2c+(3*3),T_2c,mrotc);
        double T_n2[3*4];
        LieSetIdentity<>(T_n2);
        LieSetTranslation<>(T_n2,rot_center);
        LieMulSE3(T_nc, T_n2, T_2c );
        OpenGlMatrix& spec = cam_state->GetModelViewMatrix();
        LieMul4x4bySE3<>(spec.m,T_nc,spec.m);
    }

  }



  // Use OpenGl's default frame of reference
  OpenGlMatrixSpec ProjectionMatrix(int w, int h, double fu, double fv, double u0, double v0, double zNear, double zFar )
  {
      return ProjectionMatrixRUB_BottomLeft(w,h,fu,fv,u0,v0,zNear,zFar);
  }

OpenGlMatrixSpec ProjectionMatrixOrthographic(double l, double r, double b, double t, double n, double f )
{
    OpenGlMatrixSpec P;
    P.type = GlProjectionStack;

    P.m[0] = 2/(r-l);
    P.m[1] = 0;
    P.m[2] = 0;
    P.m[3] = 0;

    P.m[4] = 0;
    P.m[5] = 2/(t-b);
    P.m[6] = 0;
    P.m[7] = 0;

    P.m[8] = 0;
    P.m[9] = 0;
    P.m[10] = -2/(f-n);
    P.m[11] = 0;

    P.m[12] = -(r+l)/(r-l);
    P.m[13] = -(t+b)/(t-b);
    P.m[14] = -(f+n)/(f-n);
    P.m[15] = 1;

    return P;
}


  // Camera Axis:
  //   X - Right, Y - Up, Z - Back
  // Image Origin:
  //   Bottom Left
  // Caution: Principal point defined with respect to image origin (0,0) at
  //          top left of top-left pixel (not center, and in different frame
  //          of reference to projection function image)
  OpenGlMatrixSpec ProjectionMatrixRUB_BottomLeft(int w, int h, double fu, double fv, double u0, double v0, double zNear, double zFar )
  {
      // http://www.songho.ca/opengl/gl_projectionmatrix.html
      const double L = +(u0) * zNear / -fu;
      const double T = +(v0) * zNear / fv;
      const double R = -(w-u0) * zNear / -fu;
      const double B = -(h-v0) * zNear / fv;

      OpenGlMatrixSpec P;
      P.type = GlProjectionStack;
      std::fill_n(P.m,4*4,0);

      P.m[0*4+0] = 2 * zNear / (R-L);
      P.m[1*4+1] = 2 * zNear / (T-B);
      P.m[2*4+2] = -(zFar +zNear) / (zFar - zNear);
      P.m[2*4+0] = (R+L)/(R-L);
      P.m[2*4+1] = (T+B)/(T-B);
      P.m[2*4+3] = -1.0;
      P.m[3*4+2] =  -(2*zFar*zNear)/(zFar-zNear);

      return P;
  }

  // Camera Axis:
  //   X - Right, Y - Down, Z - Forward
  // Image Origin:
  //   Top Left
  // Pricipal point specified with image origin (0,0) at top left of top-left pixel (not center)
  OpenGlMatrixSpec ProjectionMatrixRDF_TopLeft(int w, int h, double fu, double fv, double u0, double v0, double zNear, double zFar )
  {
      // http://www.songho.ca/opengl/gl_projectionmatrix.html
      const double L = -(u0) * zNear / fu;
      const double R = +(w-u0) * zNear / fu;
      const double T = -(v0) * zNear / fv;
      const double B = +(h-v0) * zNear / fv;

      OpenGlMatrixSpec P;
      P.type = GlProjectionStack;
      std::fill_n(P.m,4*4,0);

      P.m[0*4+0] = 2 * zNear / (R-L);
      P.m[1*4+1] = 2 * zNear / (T-B);

      P.m[2*4+0] = (R+L)/(L-R);
      P.m[2*4+1] = (T+B)/(B-T);
      P.m[2*4+2] = (zFar +zNear) / (zFar - zNear);
      P.m[2*4+3] = 1.0;

      P.m[3*4+2] =  (2*zFar*zNear)/(zNear - zFar);
      return P;
  }

  // Camera Axis:
  //   X - Right, Y - Down, Z - Forward
  // Image Origin:
  //   Bottom Left
  // Pricipal point specified with image origin (0,0) at top left of top-left pixel (not center)
  OpenGlMatrixSpec ProjectionMatrixRDF_BottomLeft(int w, int h, double fu, double fv, double u0, double v0, double zNear, double zFar )
  {
      // http://www.songho.ca/opengl/gl_projectionmatrix.html
      const double L = -(u0) * zNear / fu;
      const double R = +(w-u0) * zNear / fu;
      const double B = -(v0) * zNear / fv;
      const double T = +(h-v0) * zNear / fv;

      OpenGlMatrixSpec P;
      P.type = GlProjectionStack;
      std::fill_n(P.m,4*4,0);

      P.m[0*4+0] = 2 * zNear / (R-L);
      P.m[1*4+1] = 2 * zNear / (T-B);

      P.m[2*4+0] = (R+L)/(L-R);
      P.m[2*4+1] = (T+B)/(B-T);
      P.m[2*4+2] = (zFar +zNear) / (zFar - zNear);
      P.m[2*4+3] = 1.0;

      P.m[3*4+2] =  (2*zFar*zNear)/(zNear - zFar);
      return P;
  }

  OpenGlMatrix ModelViewLookAtRUB(double ex, double ey, double ez, double lx, double ly, double lz, double ux, double uy, double uz)
  {
    OpenGlMatrix mat;
    GLdouble* m = mat.m;

    const double u_o[3] = {ux,uy,uz};

    GLdouble x[3], y[3];
    GLdouble z[] = {ex - lx, ey - ly, ez - lz};
    Normalise<3>(z);

    CrossProduct(x,u_o,z);
    CrossProduct(y,z,x);

    Normalise<3>(x);
    Normalise<3>(y);

  #define M(row,col)  m[col*4+row]
    M(0,0) = x[0];
    M(0,1) = x[1];
    M(0,2) = x[2];
    M(1,0) = y[0];
    M(1,1) = y[1];
    M(1,2) = y[2];
    M(2,0) = z[0];
    M(2,1) = z[1];
    M(2,2) = z[2];
    M(3,0) = 0.0;
    M(3,1) = 0.0;
    M(3,2) = 0.0;
    M(0,3) = -(M(0,0)*ex + M(0,1)*ey + M(0,2)*ez);
    M(1,3) = -(M(1,0)*ex + M(1,1)*ey + M(1,2)*ez);
    M(2,3) = -(M(2,0)*ex + M(2,1)*ey + M(2,2)*ez);
    M(3,3) = 1.0;
#undef M

    return mat;
  }

  OpenGlMatrix ModelViewLookAtRDF(double ex, double ey, double ez, double lx, double ly, double lz, double ux, double uy, double uz)
  {
    OpenGlMatrix mat;
    GLdouble* m = mat.m;

    const double u_o[3] = {ux,uy,uz};

    GLdouble x[3], y[3];
    GLdouble z[] = {lx - ex, ly - ey, lz - ez};
    Normalise<3>(z);

    CrossProduct(x,z,u_o);
    CrossProduct(y,z,x);

    Normalise<3>(x);
    Normalise<3>(y);

  #define M(row,col)  m[col*4+row]
    M(0,0) = x[0];
    M(0,1) = x[1];
    M(0,2) = x[2];
    M(1,0) = y[0];
    M(1,1) = y[1];
    M(1,2) = y[2];
    M(2,0) = z[0];
    M(2,1) = z[1];
    M(2,2) = z[2];
    M(3,0) = 0.0;
    M(3,1) = 0.0;
    M(3,2) = 0.0;
    M(0,3) = -(M(0,0)*ex + M(0,1)*ey + M(0,2)*ez);
    M(1,3) = -(M(1,0)*ex + M(1,1)*ey + M(1,2)*ez);
    M(2,3) = -(M(2,0)*ex + M(2,1)*ey + M(2,2)*ez);
    M(3,3) = 1.0;
#undef M

    return mat;
  }

  OpenGlMatrix ModelViewLookAt(double ex, double ey, double ez, double lx, double ly, double lz, double ux, double uy, double uz)
  {
    return ModelViewLookAtRUB(ex,ey,ez,lz,ly,lz,ux,uy,uz);
  }

  OpenGlMatrix ModelViewLookAt(double ex, double ey, double ez, double lx, double ly, double lz, AxisDirection up)
  {
    const double* u = AxisDirectionVector[up];
    return ModelViewLookAtRUB(ex,ey,ez,lx,ly,lz,u[0],u[1],u[2]);
  }

  OpenGlMatrix IdentityMatrix()
  {
    OpenGlMatrix P;
    std::fill_n(P.m,4*4,0);
    for( int i=0; i<4; ++i ) P.m[i*4+i] = 1;
    return P;
  }

  OpenGlMatrixSpec IdentityMatrix(OpenGlStack type)
  {
    OpenGlMatrixSpec P;
    P.type = type;
    std::fill_n(P.m,4*4,0);
    for( int i=0; i<4; ++i ) P.m[i*4+i] = 1;
    return P;
  }

  OpenGlMatrixSpec  negIdentityMatrix(OpenGlStack type)
  {
    OpenGlMatrixSpec P;
    P.type = type;
    std::fill_n(P.m,4*4,0);
    for( int i=0; i<4; ++i ) P.m[i*4+i] = -1;

    P.m[3*4+3] =1;
    return P;
  }

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
