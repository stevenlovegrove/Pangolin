#ifndef PANGOLIN_GL_H
#define PANGOLIN_GL_H

#include <string>
#include "pangolin.h"

#include <GL/gl.h>

#ifdef HAVE_GLUT
#include <GL/glut.h>
#include <GL/freeglut_ext.h>
#endif

#define GLUT_KEY_ESCAPE 27
#define GLUT_KEY_TAB 9

namespace pangolin
{

  //! @brief Give this OpenGL context a name or switch contexts
  //! This is required to initialise Pangolin for use with an
  //! externally defined OpenGL context. You needn't call it
  //! if you have used CreateGlutWindowAndBind() to create a GLUT
  //! window
  void BindToContext(std::string name);

  //! @brief Returns true if user has requested to close OpenGL window.
  bool ShouldQuit();

  namespace process
  {
    //! @brief Tell pangolin to process input to drive display
    //! You will need to call this manually if you haven't let
    //! Pangolin register callbacks from your windowing system
    void Keyboard( unsigned char key, int x, int y);
  }

#ifdef HAVE_GLUT
  namespace glut
  {
    //! @brief Create GLUT window and bind Pangolin to it.
    //! All GLUT initialisation is taken care of. This prevents you
    //! from needing to call BindToContext() and TakeGlutCallbacks().
    void CreateWindowAndBind(std::string window_title, int w = 640, int h = 480 );

    //! @brief Allow Pangolin to take GLUT callbacks for its own uses
    void TakeCallbacks();
  }
#endif

  struct Viewport
  {
    GLint x,y,w,h;
    void Activate();
  };

  struct Projection
  {
    GLfloat m[16];
  };

  struct Display
  {
    Viewport v;
    Projection p;
  };

}


#endif // PANGOLIN_GL_H

