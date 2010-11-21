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
    }
    context = ic->second;
  }

  bool ShouldQuit()
  {
    return context->quit;
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
      glutCreateWindow(window_title.c_str());
      BindToContext(window_title);
      TakeCallbacks();
    }

    void TakeCallbacks()
    {
      glutKeyboardFunc(&process::Keyboard);
    }

  }
#endif

  void Viewport::Activate()
  {
    glViewport(x,y,w,h);
  }


}
