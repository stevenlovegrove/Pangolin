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

#ifndef PANGOLIN_DISPLAY_H
#define PANGOLIN_DISPLAY_H

#include "platform.h"
#include <string>
#include <map>
#include <vector>
#include <cmath>
#include <iostream>
#include <boost/function.hpp>

#ifdef HAVE_GLUT

#ifdef HAVE_APPLE_OPENGL_FRAMEWORK
    #include <GLUT/glut.h>
    #define HAVE_GLUT_APPLE_FRAMEWORK

    inline void glutBitmapString(void* font, const unsigned char* str)
    {
        const unsigned char* s = str;
        while(*s != 0) {
            glutBitmapCharacter(font, *s);
            ++s;
        }
    }
#else
    #include <GL/freeglut.h>
#endif

#endif // HAVE_GLUT

#ifdef HAVE_TOON
#include <cstring>
#include <TooN/TooN.h>
#include <TooN/se3.h>
#endif

#ifdef HAVE_EIGEN
#include <Eigen/Eigen>
#endif

#ifdef _WIN_
#include <Windows.h>
#endif

#include <GL/gl.h>

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

  //! @brief Request that the program exit
  void Quit();

  //! @brief Returns true if user has requested to close OpenGL window.
  bool ShouldQuit();

  //! @brief Returns true if user has interacted with the window since this was last called
  bool HadInput();

  //! @brief Returns true if user has resized the window
  bool HasResized();

  //! @brief Renders any views with default draw methods
  void RenderViews();

  //! @brief Request to be notified via functor when key is pressed.
  //! Functor may take one parameter which will equal the key pressed
  void RegisterKeyPressCallback(int key, boost::function<void(void)> func);

  // Supported Key modifiers for GlobalKeyPressCallback.
  // e.g. PANGO_CTRL + 'r', PANGO_SPECIAL + GLUT_KEY_RIGHT, etc.
  const int PANGO_SPECIAL = 128;
  const int PANGO_CTRL = -96;
  const int PANGO_OPTN = 132;

  namespace process
  {
    //! @brief Tell pangolin to process input to drive display
    //! You will need to call this manually if you haven't let
    //! Pangolin register callbacks from your windowing system
    void Keyboard( unsigned char key, int x, int y);

    void KeyboardUp(unsigned char key, int x, int y);

    //! @brief Tell pangolin base window size has changed
    //! You will need to call this manually if you haven't let
    //! Pangolin register callbacks from your windowing system
    void Resize(int width, int height);

    void Mouse( int button, int state, int x, int y);

    void MouseMotion( int x, int y);
  }

#ifdef HAVE_GLUT  
  //! @brief Create GLUT window and bind Pangolin to it.
  //! All GLUT initialisation is taken care of. This prevents you
  //! from needing to call BindToContext() and TakeGlutCallbacks().
  void CreateGlutWindowAndBind(std::string window_title, int w = 640, int h = 480, unsigned int mode = GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );

  //! @brief Applies any post-render events if they are defined,
  //! swaps buffers and processes events. Also resets viewport to
  //! entire window and disables scissoring.
  void FinishGlutFrame();

  //! @brief Swaps OpenGL Buffers and processes input events
  void SwapGlutBuffersProcessGlutEvents();

  //! @brief Allow Pangolin to take GLUT callbacks for its own uses
  //! Not needed if you instantiated a window through CreateWindowAndBind().
  void TakeGlutCallbacks();
#endif

  //! @brief Unit for measuring quantities
  enum Unit {
    Fraction,
    Pixel,
    ReversePixel
  };

  //! @brief Defines typed quantity
  //! Constructors distinguised by whole pixels, or floating
  //! fraction in interval [0,1]
  struct Attach {
    Attach() : unit(Fraction), p(0) {}
    Attach(Unit unit, GLfloat p) : unit(unit), p(p) {}

    Attach(GLfloat p) : unit(Fraction), p(p) {
        if( p < 0 || 1.0 < p ) {
            std::cerr << "Pangolin API Change: Display::SetBounds must be used with Attach::Pix or Attach::ReversePix to specify pixel bounds relative to an edge. See the code samples for details." << std::endl;
            throw std::exception();
        }
    }

//    Attach(GLdouble p) : unit(Fraction), p(p) {}

    static Attach Pix(int p) {
        return Attach(p >=0 ? Pixel : ReversePixel, std::abs((float)p));
    }
    static Attach ReversePix(int p) {
        return Attach(ReversePixel, p);
    }
    static Attach Frac(float frac) {
        return Attach(frac);
    }

    Unit unit;
    GLfloat p;

//  protected:
//    Attach(int p) {}
  };


  enum Lock {
    LockLeft = 0,
    LockBottom = 0,
    LockCenter = 1,
    LockRight = 2,
    LockTop = 2
  };

  //! @brief Encapsulates OpenGl Viewport
  struct Viewport
  {
    Viewport() {}
    Viewport(GLint l,GLint b,GLint w,GLint h);
    void Activate() const;
    void Scissor() const;
    void ActivateAndScissor() const;
    bool Contains(int x, int y) const;

    Viewport Inset(int i) const;
    Viewport Inset(int horiz, int vert) const;

    static void DisableScissor();

    GLint r() const;
    GLint t() const;
    GLfloat aspect() const;
    GLint l,b,w,h;
  };

  //! @brief Capture OpenGL matrix types in enum to typing
  enum OpenGlStack {
    GlProjectionStack = GL_PROJECTION,
    GlModelViewStack = GL_MODELVIEW,
    GlTextureStack = GL_TEXTURE
  };

  struct CameraSpec {
    GLdouble forward[3];
    GLdouble up[3];
    GLdouble right[3];
    GLdouble img_up[2];
    GLdouble img_right[2];
  };

  const static CameraSpec CameraSpecOpenGl = {{0,0,-1},{0,1,0},{1,0,0},{0,1},{1,0}};
  const static CameraSpec CameraSpecYDownZForward = {{0,0,1},{0,-1,0},{1,0,0},{0,-1},{1,0}};

  //! @brief Object representing OpenGl Matrix
  struct OpenGlMatrix {
    OpenGlMatrix();

#ifdef HAVE_EIGEN
    template<typename T>
    OpenGlMatrix(const Eigen::Matrix<T,4,4>& mat);
#endif // HAVE_EIGEN

    // Load matrix on to OpenGl stack
    void Load() const;

    void Multiply() const;

    void SetIdentity();

    // Column major Internal buffer
    GLdouble m[16];
  };

  //! @brief deprecated
  struct OpenGlMatrixSpec : public OpenGlMatrix {
    // Specify which stack this refers to
    OpenGlStack type;
  };

  //! @brief Object representing attached OpenGl Matrices / transforms
  //! Only stores what is attached, not entire OpenGl state (which would
  //! be horribly slow). Applying state is efficient.
  struct OpenGlRenderState
  {
    OpenGlRenderState();
    OpenGlRenderState(const OpenGlMatrix& projection_matrix);
    OpenGlRenderState(const OpenGlMatrix& projection_matrix, const OpenGlMatrix& modelview_matrix);

    static void ApplyIdentity();
    static void ApplyWindowCoords();

    void Apply() const;
    OpenGlRenderState& SetProjectionMatrix(OpenGlMatrix spec);
    OpenGlRenderState& SetModelViewMatrix(OpenGlMatrix spec);

    //! deprecated
    OpenGlRenderState& Set(OpenGlMatrixSpec spec);

    std::map<OpenGlStack,OpenGlMatrix> stacks;
  };

  enum Layout
  {
    LayoutOverlay,
    LayoutVertical,
    LayoutHorizontal,
    LayoutEqual
  };

  // Forward declaration
  struct Handler;

  //! @brief A Display manages the location and resizing of an OpenGl viewport.
  struct View
  {
    View()
      : aspect(0.0), top(1.0),left(0.0),right(1.0),bottom(0.0), hlock(LockCenter),vlock(LockCenter),
        layout(LayoutOverlay), scroll_offset(0), show(1), handler(0) {}

    virtual ~View() {}

    //! Activate Displays viewport for drawing within this area
    void Activate() const;

    //! Activate Displays and set State Matrices
    void Activate(const OpenGlRenderState& state ) const;

    //! Activate Displays viewport and Scissor for drawing within this area
    void ActivateAndScissor() const;

    //! Activate Displays viewport and Scissor for drawing within this area
    void ActivateScissorAndClear() const;

    //! Activate Display and set State Matrices
    void ActivateAndScissor(const OpenGlRenderState& state ) const;

    //! Activate Display and set State Matrices
    void ActivateScissorAndClear(const OpenGlRenderState& state ) const;

    //! Given the specification of Display, compute viewport
    virtual void Resize(const Viewport& parent);

    //! Instruct all children to resize
    virtual void ResizeChildren();

    //! Perform any automatic rendering for this View.
    //! Default implementation simply instructs children to render themselves.
    virtual void Render();

    //! Instruct all children to render themselves if appropriate
    virtual void RenderChildren();

    //! Set this view as the active View to receive input
    View& SetFocus();

    //! Set bounds for the View using mixed fractional / pixel coordinates (OpenGl view coordinates)
    View& SetBounds(Attach bottom, Attach top, Attach left, Attach right, bool keep_aspect = false);

    //! Set bounds for the View using mixed fractional / pixel coordinates (OpenGl view coordinates)
    View& SetBounds(Attach bottom, Attach top, Attach left, Attach right, double aspect);

    View& SetHandler(Handler* handler);
    View& SetAspect(double aspect);
    View& SetLock(Lock horizontal, Lock vertical );
    View& SetLayout(Layout layout);
    View& AddDisplay(View& view);

    //! Return (i)th child of this view
    View& operator[](int i);

    // Desired width / height aspect (0 if dynamic)
    double aspect;

    // Bounds to fit display within
    Attach top, left, right, bottom;
    Lock hlock;
    Lock vlock;
    Layout layout;

    int scroll_offset;

    // Cached client area (space allocated from parent)
    Viewport vp;

    // Cached absolute viewport (recomputed on resize - respects aspect)
    Viewport v;

    // Should this view be displayed?
    bool show;

    // Input event handler (if any)
    Handler* handler;

    // Map for sub-displays (if any)
    std::vector<View*> views;

  private:
    // Private copy constructor
    View(View&) { /* Do Not copy - take reference instead*/ }
  };

  enum MouseButton
  {
    MouseButtonLeft = 1,
    MouseButtonMiddle = 2,
    MouseButtonRight = 4,
    MouseWheelUp = 8,
    MouseWheelDown = 16
  };

  //! @brief Input Handler base class with virtual methods which recurse
  //! into sub-displays
  struct Handler
  {
    virtual ~Handler() {}
    virtual void Keyboard(View&, unsigned char key, int x, int y, bool pressed);
    virtual void Mouse(View&, MouseButton button, int x, int y, bool pressed, int button_state);
    virtual void MouseMotion(View&, int x, int y, int button_state);
  };
  static Handler StaticHandler;

  struct HandlerScroll : Handler
  {
    void Mouse(View&, MouseButton button, int x, int y, bool pressed, int button_state);
  };
  static HandlerScroll StaticHandlerScroll;

  enum AxisDirection
  {
      AxisNone,
      AxisNegX, AxisX,
      AxisNegY, AxisY,
      AxisNegZ, AxisZ
  };

  struct Handler3D : Handler
  {

    Handler3D(OpenGlRenderState& cam_state, AxisDirection enforce_up=AxisNone, float trans_scale=0.01f)
        : cam_state(&cam_state), enforce_up(enforce_up), tf(trans_scale), cameraspec(CameraSpecOpenGl), last_z(1.0) {}

    void SetOpenGlCamera();
    void Mouse(View&, MouseButton button, int x, int y, bool pressed, int button_state);
    void MouseMotion(View&, int x, int y, int button_state);

  protected:
    OpenGlRenderState* cam_state;
    const static int hwin = 3;
    AxisDirection enforce_up;
    float tf;
    CameraSpec cameraspec;
    GLfloat last_z;
    GLint last_pos[2];
    GLdouble rot_center[3];
  };

  //! Retrieve 'base' display, corresponding to entire window
  View& DisplayBase();

  //! Create or retrieve named display
  View& Display(const std::string& name);

  //! Create unnamed display
  View& CreateDisplay();

  OpenGlMatrixSpec ProjectionMatrixRUB_BottomLeft(int w, int h, double fu, double fv, double u0, double v0, double zNear, double zFar );
  OpenGlMatrixSpec ProjectionMatrixRDF_TopLeft(int w, int h, double fu, double fv, double u0, double v0, double zNear, double zFar );
  OpenGlMatrixSpec ProjectionMatrixRDF_BottomLeft(int w, int h, double fu, double fv, double u0, double v0, double zNear, double zFar );

  // Use OpenGl's default frame RUB_BottomLeft
  OpenGlMatrixSpec ProjectionMatrix(int w, int h, double fu, double fv, double u0, double v0, double zNear, double zFar );

  OpenGlMatrix IdentityMatrix();
  OpenGlMatrixSpec IdentityMatrix(OpenGlStack type);
  OpenGlMatrixSpec negIdentityMatrix(OpenGlStack type);

#ifdef HAVE_TOON
  OpenGlMatrixSpec FromTooN(const TooN::SE3<>& T_cw);
  OpenGlMatrixSpec FromTooN(OpenGlStack type, const TooN::Matrix<4,4>& M);
  TooN::Matrix<4,4> ToTooN(const OpenGlMatrixSpec& ms);
  TooN::SE3<> ToTooN_SE3(const OpenGlMatrixSpec& ms);
#endif


}

// Inline definitions
namespace pangolin
{

inline Viewport::Viewport(GLint l,GLint b,GLint w,GLint h) : l(l),b(b),w(w),h(h) {}
inline GLint Viewport::r() const { return l+w;}
inline GLint Viewport::t() const { return b+h;}
inline GLfloat Viewport::aspect() const { return (GLfloat)w / (GLfloat)h; }


inline OpenGlMatrix::OpenGlMatrix() {
}

#ifdef HAVE_EIGEN
  template<typename T> inline
  OpenGlMatrix::OpenGlMatrix(const Eigen::Matrix<T,4,4>& mat)
  {
      for(int r=0; r<4; ++r ) {
          for(int c=0; c<4; ++c ) {
              m[c*4+r] = mat(r,c);
          }
      }
  }

#endif

#ifdef HAVE_TOON

inline OpenGlMatrixSpec FromTooN(const TooN::SE3<>& T_cw)
{
    TooN::Matrix<4,4,double,TooN::ColMajor> M;
    M.slice<0,0,3,3>() = T_cw.get_rotation().get_matrix();
    M.T()[3].slice<0,3>() = T_cw.get_translation();
    M[3] = TooN::makeVector(0,0,0,1);

    OpenGlMatrixSpec P;
    P.type = GlModelViewStack;
    std::memcpy(P.m, &(M[0][0]),16*sizeof(double));
    return P;
}

inline OpenGlMatrixSpec FromTooN(OpenGlStack type, const TooN::Matrix<4,4>& M)
{
    // Read in remembering col-major convension for our matrices
    OpenGlMatrixSpec P;
    P.type = type;
    int el = 0;
    for(int c=0; c<4; ++c)
        for(int r=0; r<4; ++r)
            P.m[el++] = M[r][c];
    return P;
}

inline TooN::Matrix<4,4> ToTooN(const OpenGlMatrixSpec& ms)
{
    TooN::Matrix<4,4> m;
    int el = 0;
    for( int c=0; c<4; ++c )
        for( int r=0; r<4; ++r )
            m(r,c) = ms.m[el++];
    return m;
}

inline TooN::SE3<> ToTooN_SE3(const OpenGlMatrixSpec& ms)
{
    TooN::Matrix<4,4> m = ToTooN(ms);
    const TooN::SO3<> R(m.slice<0,0,3,3>());
    const TooN::Vector<3> t = m.T()[3].slice<0,3>();
    return TooN::SE3<>(R,t);
}


#endif

}

#endif // PANGOLIN_DISPLAY_H

