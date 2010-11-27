#include <iostream>
#include <map>

#include "platform.h"
#include "display.h"
#include "display_internal.h"
#include "simple_math.h"

using namespace std;

namespace pangolin
{
  typedef map<string,PangolinGl*> ContextMap;

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
      ic = contexts.insert( pair<string,PangolinGl*>(name,new PangolinGl) ).first;
      context = ic->second;
      View& dc = context->base;
      dc.left = 0;
      dc.bottom = 0;
      dc.top = 1.0;
      dc.right = 1.0;
      dc.aspect = 0;
      dc.handler = &StaticHandler;
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
    return context->quit;
  }

  bool HadInput()
  {
    const bool hi = context->had_input;
    context->had_input = false;
    return hi;
  }

  bool HasResized()
  {
    const bool hr = context->has_resized;
    context->has_resized = false;
    return hr;
  }

  View& DisplayBase()
  {
    return context->base;
  }

  View& Display(const std::string& name)
  {
    // Get / Create View
    std::map<std::string,View*>::iterator vi = context->all_views.find(name);
    if( vi != context->all_views.end() )
    {
      return *(vi->second);
    }else{
      View* v = new View();
      context->all_views[name] = v;
//      context->base[name] = v;
      context->base.views.push_back(v);
      return *v;
    }
  }

  namespace process
  {
    void Keyboard( unsigned char key, int x, int y)
    {
      context->had_input = true;

      // Force coords to match OpenGl Window Coords
      y = context->base.v.h - y;

      if( key == GLUT_KEY_TAB)
        glutFullScreenToggle();
      else if( key == GLUT_KEY_ESCAPE)
        context->quit = true;
      else
      {
        context->base.handler->Keyboard(context->base,key,x,y);
      }
    }

    void Mouse( int button, int state, int x, int y)
    {
      context->had_input = true;

      // Force coords to match OpenGl Window Coords
      y = context->base.v.h - y;

      const bool fresh_input = (context->mouse_state == 0);

      if( state == 0 )
      {
        // mouse down
        context->mouse_state |= 1 << button;
      }else if( state == 1 )
      {
        // mouse up
        context->mouse_state &= ~(1 << button);
      }

      if(fresh_input)
      {
        context->base.handler->Mouse(context->base,button,state,x,y);
      }else if(context->activeDisplay && context->activeDisplay->handler)
      {
        context->activeDisplay->handler->Mouse(*(context->activeDisplay),button,state,x,y);
      }

      if(context->mouse_state == 0 )
      {
        context->activeDisplay = 0;
      }
    }

    void MouseMotion( int x, int y)
    {
      context->had_input = true;

      // Force coords to match OpenGl Window Coords
      y = context->base.v.h - y;

      if( context->activeDisplay)
      {
        if( context->activeDisplay->handler )
          context->activeDisplay->handler->MouseMotion(*(context->activeDisplay),x,y);
      }else{
        context->base.handler->MouseMotion(context->base,x,y);
      }
    }

    void Resize( int width, int height )
    {
      context->had_input = true;
      context->has_resized = true;
      Viewport win(0,0,width,height);
      context->base.Resize(win);
    }
  }

#ifdef HAVE_GLUT
  void TakeGlutCallbacks()
  {
    glutKeyboardFunc(&process::Keyboard);
    glutReshapeFunc(&process::Resize);
    glutMouseFunc(&process::Mouse);
    glutMotionFunc(&process::MouseMotion);
  }

  void CreateGlutWindowAndBind(string window_title, int w, int h)
  {
    if( glutGet(GLUT_INIT_STATE) == 0)
    {
      int argc = 0;
      glutInit(&argc, 0);
      glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    }
    glutInitWindowSize(w,h);
    glutCreateWindow(window_title.c_str());
    BindToContext(window_title);
    TakeGlutCallbacks();
  }
#endif

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

  void OpenGlMatrixSpec::Load() const
  {
    glMatrixMode(type);
    glLoadMatrixd(m);
  }

  void OpenGlRenderState::Apply() const
  {
    // Apply any stack matrices we have
    for(map<OpenGlStack,OpenGlMatrixSpec>::const_iterator i = stacks.begin(); i != stacks.end(); ++i )
    {
      i->second.Load();
    }

    // Leave in MODEVIEW mode
    glMatrixMode(GL_MODELVIEW);
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


  OpenGlRenderState& OpenGlRenderState::Set(OpenGlMatrixSpec spec)
  {
    stacks[spec.type] = spec;
    return *this;
  }

  int AttachAbs( int low, int high, Attach a)
  {
    if( a.unit == Pixel ) return low + a.p;
    if( a.unit == ReversePixel ) return high - a.p;
    return low + a.p * (high - low);
  }

  void View::Resize(const Viewport& p)
  {
    // Compute Bounds based on specification
    v.l = AttachAbs(p.l,p.r(),left);
    v.b = AttachAbs(p.b,p.t(),bottom);
    const int r = AttachAbs(p.l,p.r(),right);
    const int t = AttachAbs(p.b,p.t(),top);
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
      for( vector<View*>::const_iterator i = views.begin(); i != views.end(); ++i )
        (*i)->Resize(v);
    }else if( layout == LayoutVertical )
    {
      // Allocate space incrementally
      const int margin = 8;
      Viewport space = v.Inset(margin);
      for( vector<View*>::const_iterator i = views.begin(); i != views.end(); ++i )
      {
        (*i)->Resize(space);
        space.h = (*i)->v.b - margin - space.b;
      }
    }
  }

  void View::Render()
  {
    RenderChildren();
  }

  void View::RenderChildren()
  {
    for( vector<View*>::const_iterator i = views.begin(); i != views.end(); ++i )
      (*i)->Render();
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


  View& View::SetBounds(Attach top, Attach bottom,  Attach left, Attach right, bool keep_aspect)
  {
    SetBounds(top,bottom,left,right,0.0);
    aspect = keep_aspect ? v.aspect() : 0;
    return *this;
  }

  View& View::SetBounds(Attach top, Attach bottom,  Attach left, Attach right, double aspect)
  {
    this->left = left;
    this->top = top;
    this->right = right;
    this->bottom = bottom;
    this->aspect = aspect;
    this->Resize(context->base.v);
    return *this;
  }

  View& View::SetAspect(double aspect)
  {
    this->aspect = aspect;
    this->Resize(context->base.v);
    return *this;
  }

  View& View::SetLock(Lock horizontal, Lock vertical )
  {
    vlock = vertical;
    hlock = horizontal;
    return *this;
  }

  View& View::SetHandler(Handler* h)
  {
    handler = h;
    return *this;
  }

  View* FindChild(View& parent, int x, int y)
  {
    for( vector<View*>::const_iterator i = parent.views.begin(); i != parent.views.end(); ++i )
      if( (*i)->vp.Contains(x,y) )
        return (*i);
    return 0;
  }

  void Handler::Keyboard(View& d, unsigned char key, int x, int y)
  {
    View* child = FindChild(d,x,y);
    if( child)
    {
      context->activeDisplay = child;
      if( child->handler)
        child->handler->Keyboard(*child,key,x,y);
    }
  }

  void Handler::Mouse(View& d, int button, int state, int x, int y)
  {
    View* child = FindChild(d,x,y);
    if( child )
    {
      context->activeDisplay = child;
      if( child->handler)
        child->handler->Mouse(*child,button,state,x,y);
    }
  }

  void Handler::MouseMotion(View& d, int x, int y)
  {
    View* child = FindChild(d,x,y);
    if( child )
    {
      context->activeDisplay = child;
      if( child->handler)
        child->handler->MouseMotion(*child,x,y);
    }
  }

  void Handler3D::Mouse(View& display, int button, int state, int x, int y)
  {
    // mouse down
    last_pos[0] = x;
    last_pos[1] = y;

    double T_nc[3*4];
    LieSetIdentity(T_nc);

    if( state == 0 )
    {
      OpenGlMatrixSpec& proj = cam_state->stacks[GlProjection];
      // TODO: check it actually exists!

      // Find 3D point using depth buffer
      glReadBuffer(GL_FRONT);
      GLint viewport[4] = {display.v.l,display.v.b,display.v.w,display.v.h};
      const int zl = (hwin*2+1);
      const int zsize = zl*zl;
      GLfloat zs[zsize];
      glReadPixels(x-hwin,y-hwin,zl,zl,GL_DEPTH_COMPONENT,GL_FLOAT,zs);
      last_z = *(std::min_element(zs,zs+zsize));

      if( last_z != 1 )
      {
        gluUnProject(x,y,last_z,Identity4d,proj.m,viewport,rot_center,rot_center+1,rot_center+2);
      }else{
        // Points on far clipping plane are invalid
        SetZero<3,1>(rot_center);
      }

      // Wheel
      if( button == 3 || button == 4)
      {
        LieSetIdentity(T_nc);
        const double t[] = { 0,0,(button==3?1:-1)*100*tf};
        LieSetTranslation<>(T_nc,t);
        if( !(context->mouse_state & (1<<2)) && !(rot_center[0]==0 && rot_center[1]==0 && rot_center[2]==0) )
        {
          LieSetTranslation<>(T_nc,rot_center);
          MatMul<3,1>(T_nc+(3*3),(button==3?-1.0:1.0)/5.0);
        }
        OpenGlMatrixSpec& spec = cam_state->stacks[GlModelView];
        LieMul4x4bySE3<>(spec.m,T_nc,spec.m);
      }
    }
  }

  void Handler3D::MouseMotion(View& display, int x, int y)
  {
    const int delta[2] = {(x-last_pos[0]),(y-last_pos[1])};
    const float mag = delta[0]*delta[0] + delta[1]*delta[1];

    // TODO: convert delta to degrees based of fov
    // TODO: make transformation with respect to cam spec

    if( mag < 50*50 )
    {
      double T_nc[3*4];
      LieSetIdentity(T_nc);

      if( context->mouse_state == 2 )
      {
        // Middle Drag: in plane translate
        Rotation<>(T_nc,-delta[1]*0.01, -delta[0]*0.01, 0.0);
      }else if( context->mouse_state == 1 )
      {
        // Left Drag: in plane translate
        if( last_z != 1 )
        {
          //TODO Check proj exists
          OpenGlMatrixSpec& proj = cam_state->stacks[GlProjection];
          GLint viewport[4] = {display.v.l,display.v.b,display.v.w,display.v.h};
          GLdouble np[2];
          gluUnProject(x,y,last_z,Identity4d,proj.m,viewport,np,np+1,np+2);
          const double t[] = { np[0] - rot_center[0], np[1] - rot_center[1], 0};
          LieSetTranslation<>(T_nc,t);
          std::copy(np,np+3,rot_center);
        }else{
          const double t[] = { -10*delta[0]*tf, 10*delta[1]*tf, 0};
          LieSetTranslation<>(T_nc,t);
        }
      }else if( context->mouse_state == 5)
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

      }else if( context->mouse_state == 4)
      {
        // Right Drag: object centric rotation
        double T_2c[3*4];
        Rotation<>(T_2c,-delta[1]*0.01, -delta[0]*0.01, 0.0);
        double mrotc[3];
        MatMul<3,1>(mrotc, rot_center, -1.0);
        LieApplySO3<>(T_2c+(3*3),T_2c,mrotc);
        double T_n2[3*4];
        LieSetIdentity<>(T_n2);
        LieSetTranslation<>(T_n2,rot_center);
        LieMulSE3(T_nc, T_n2, T_2c );
      }

      OpenGlMatrixSpec& spec = cam_state->stacks[GlModelView];
      LieMul4x4bySE3<>(spec.m,T_nc,spec.m);
    }

    last_pos[0] = x;
    last_pos[1] = y;
  }

  OpenGlMatrixSpec ProjectionMatrix(int w, int h, double fu, double fv, double u0, double v0, double zNear, double zFar )
  {
      // http://www.songho.ca/opengl/gl_projectionmatrix.html
      const double L = +(u0) * zNear / fu;
      const double T = +(v0) * zNear / fv;
      const double R = -(w-u0) * zNear / fu;
      const double B = -(h-v0) * zNear / fv;

      OpenGlMatrixSpec P;
      P.type = GlProjection;
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

  OpenGlMatrixSpec IdentityMatrix(OpenGlStack type)
  {
    OpenGlMatrixSpec P;
    P.type = type;
    std::fill_n(P.m,4*4,0);
    for( int i=0; i<4; ++i ) P.m[i*4+i] = 1;
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
