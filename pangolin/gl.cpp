#define PANGOLIN_GL_CPP

#include <iostream>
#include <map>

#include "platform.h"
#include "gl.h"
#include "gl_internal.h"

using namespace std;

namespace pangolin
{
  typedef map<string,PangolinGl*> ContextMap;

  // Map of active contexts
  ContextMap contexts;

  // Context active for current thread
  __thread PangolinGl* context = 0;

  PangolinGl::PangolinGl()
   : quit(false)
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
      DisplayContainer& dc = context->base;
      dc.left = 0;
      dc.bottom = 0;
      dc.top = 1.0;
      dc.right = 1.0;
      dc.aspect = 0;
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

  bool ShouldQuit()
  {
    return context->quit;
  }

  Display* AddDisplay(string name, Attach top, Attach left, Attach bottom, Attach right, bool keep_aspect )
  {
    Display* d = AddDisplay(name,top,left,bottom,right, 0.0f );
    d->aspect = keep_aspect ? d->v.aspect() : 0;
    return d;
  }

  Display* AddDisplay(std::string name, Attach top, Attach left, Attach bottom, Attach right, float aspect )
  {
    Display* d = new Display();
    d->left = left;
    d->top = top;
    d->right = right;
    d->bottom = bottom;
    d->aspect = aspect;
    d->RecomputeViewport(context->base.v);
    context->base[name] = d;
    return d;
  }

  Display*& GetDisplay(string name)
  {
    return context->base[name];
  }


  namespace process
  {
    void Keyboard( unsigned char key, int x, int y)
    {
      //  int mod = glutGetModifiers();

        if( key == GLUT_KEY_TAB)
          glutFullScreenToggle();
        else if( key == GLUT_KEY_ESCAPE)
          context->quit = true;
        else if( key == ' ')
        {
          const int w = glutGet(GLUT_WINDOW_WIDTH);
          const int h = glutGet(GLUT_WINDOW_HEIGHT);
          cout << w << "x" << h << endl;
        }
    }

    void Resize( int width, int height )
    {
      Viewport win(0,0,width,height);
      context->base.RecomputeViewport(win);
    }
  }

#ifdef HAVE_GLUT
  namespace glut
  {

    void CreateWindowAndBind(string window_title, int w, int h)
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
      TakeCallbacks();
    }

    void TakeCallbacks()
    {
      glutKeyboardFunc(&process::Keyboard);
      glutReshapeFunc(&process::Resize);
    }

  }
#endif

  void Viewport::Activate()
  {
    glViewport(l,b,w,h);
  }

  void Display::RecomputeViewport(const Viewport& p)
  {
    // Compute Bounds based on specification
    v.l = p.l + (left.unit == Pixel ) ? (left.p) : ( left.p * (p.r()-p.l) );
    v.b = p.b + (bottom.unit == Pixel ) ? (bottom.p) : ( bottom.p * (p.t()-p.b) );
    const int r = p.l + (right.unit == Pixel ) ? (right.p) : ( right.p * (p.r()-p.l) );
    const int t = p.b + (top.unit == Pixel ) ? (top.p) : ( top.p * (p.t()-p.b) );
    v.w = r - v.l;
    v.h = t - v.b;

    // Adjust based on aspect requirements
    if( aspect != 0 )
    {
      const float current_aspect = (float)v.w / (float)v.h;
      if( current_aspect < aspect )
      {
        //Adjust height
        const int nh = (int)(v.w / aspect);
        v.b += (v.h-nh)/2;
        v.h = nh;
      }else if( current_aspect > aspect )
      {
        //Adjust width
        const int nw = (int)(v.h * aspect);
        v.l += (v.w-nw)/2;
        v.w = nw;
      }
    }
  }

  void Display::Activate()
  {
    v.Activate();
  }

  Display*& DisplayContainer::operator[](std::string name)
  {
    return displays[name];
  }

  void DisplayContainer::RecomputeViewport(const Viewport& parent)
  {
    Display::RecomputeViewport(parent);
    for( map<string,Display*>::const_iterator i = displays.begin(); i != displays.end(); ++i )
    {
      i->second->RecomputeViewport(v);
    }
  }


}
