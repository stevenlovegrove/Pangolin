#ifndef PANGOLIN_GL_H
#define PANGOLIN_GL_H

#include <map>
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

    //! @brief Tell pangolin base window size has changed
    //! You will need to call this manually if you haven't let
    //! Pangolin register callbacks from your windowing system
    void Resize(int width, int height);
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

  //! @brief Unit for measuring quantities
  enum Unit {
    Fraction,
    Pixel
  };

  //! @brief Defines typed quantity
  //! Constructors distinguised by whole pixels, or floating fraction
  struct Attach {
    Attach() : unit(Fraction), p(0) {}
    Attach(int p) : unit(Pixel), p(p) {}
    Attach(GLfloat p) : unit(Fraction), p(p) {}
    Attach(GLdouble p) : unit(Fraction), p(p) {}
    Unit unit;
    GLfloat p;
  };

  //! @brief Encapsulates OpenGl Viewport
  struct Viewport
  {
    Viewport() {}
    Viewport(GLint l,GLint b,GLint w,GLint h);
    void Activate();
    GLint r() const;
    GLint t() const;
    GLfloat aspect() const;
    GLint l,b,w,h;
  };

  struct Projection
  {
    GLfloat m[16];
  };

  //! @brief A Displays manages the location and resizing of a viewport.
  struct Display
  {
    //! Activate Displays viewport for drawing within this area
    virtual void Activate();

    //! Given the specification of Display, compute viewport
    virtual void RecomputeViewport(const Viewport& parent);

    Projection p;

    // Desired width / height aspect (0 if dynamic)
    float aspect;

    // Bounds to fit display within
    Attach top, left, right, bottom;

    // Cached absolute viewport (recomputed on resize)
    Viewport v;
  };

  //! @brief Container for Displays
  struct DisplayContainer : public Display
  {
    void RecomputeViewport(const Viewport& parent);

    Display*& operator[](std::string name);
    std::map<std::string,Display*> displays;
  };

  Display* AddDisplay(std::string name, Attach top, Attach left, Attach bottom, Attach right, bool keep_aspect = false );
  Display* AddDisplay(std::string name, Attach top, Attach left, Attach bottom, Attach right, float aspect );
  Display*& GetDisplay(std::string name);

}

inline pangolin::Viewport::Viewport(GLint l,GLint b,GLint w,GLint h) : l(l),b(b),w(w),h(h) {};
inline GLint pangolin::Viewport::r() const { return l+w;}
inline GLint pangolin::Viewport::t() const { return b+h;}
inline GLfloat pangolin::Viewport::aspect() const { return (GLfloat)w / (GLfloat)h; }


#endif // PANGOLIN_GL_H

