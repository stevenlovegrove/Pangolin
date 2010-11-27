#ifndef PANGOLIN_DISPLAY_H
#define PANGOLIN_DISPLAY_H

#include "platform.h"
#include <string>
#include <map>
#include <vector>
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

  //! @brief Request that the program exit
  void Quit();

  //! @brief Returns true if user has requested to close OpenGL window.
  bool ShouldQuit();

  //! @brief Returns true if user has interacted with the window since this was last called
  bool HadInput();

  //! @brief Returns true if user has resized the window
  bool HasResized();

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

    void Mouse( int button, int state, int x, int y);

    void MouseMotion( int x, int y);
  }

#ifdef HAVE_GLUT
  //! @brief Create GLUT window and bind Pangolin to it.
  //! All GLUT initialisation is taken care of. This prevents you
  //! from needing to call BindToContext() and TakeGlutCallbacks().
  void CreateGlutWindowAndBind(std::string window_title, int w = 640, int h = 480 );

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
    Attach(int p) : unit(p >=0 ? Pixel : ReversePixel), p(abs(p)) {}
    Attach(GLfloat p) : unit(Fraction), p(p) {}
    Attach(GLdouble p) : unit(Fraction), p(p) {}
    Attach(int p, bool reverse) : unit(ReversePixel), p(p) {}
    Unit unit;
    GLfloat p;
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
  struct OpenGlMatrixSpec {
    // Load matrix on to OpenGl stack
    void Load() const;

    // Specify which stack this refers to
    OpenGlStack type;

    // Column major Internal buffer
    GLdouble m[16];
  };

  //! @brief Object representing attached OpenGl Matrices / transforms
  //! Only stores what is attached, not entire OpenGl state (which would
  //! be horribly slow). Applying state is efficient.
  struct OpenGlRenderState
  {
    static void ApplyIdentity();
    static void ApplyWindowCoords();

    void Apply() const;
    OpenGlRenderState& Set(OpenGlMatrixSpec spec);
    std::map<OpenGlStack,OpenGlMatrixSpec> stacks;
  };

  enum Layout
  {
    LayoutOverlay,
    LayoutVertical,
    LayoutHorizontal
  };

  // Forward declaration
  struct Handler;

  //! @brief A Display manages the location and resizing of an OpenGl viewport.
  struct View
  {
    View()
      : aspect(0.0), top(1.0),left(0),right(1.0),bottom(0), hlock(LockCenter),vlock(LockCenter),
        layout(LayoutOverlay), handler(0) {}

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


    //! Set bounds for the View using mixed fractional / pixel coordinates (OpenGl view coordinates)
    View& SetBounds(Attach top, Attach bottom,  Attach left, Attach right, bool keep_aspect = false);

    //! Set bounds for the View using mixed fractional / pixel coordinates (OpenGl view coordinates)
    View& SetBounds(Attach top, Attach bottom,  Attach left, Attach right, double aspect);

    View& SetHandler(Handler* handler);
    View& SetAspect(double aspect);
    View& SetLock(Lock horizontal, Lock vertical );

    // Desired width / height aspect (0 if dynamic)
    double aspect;

    // Bounds to fit display within
    Attach top, left, right, bottom;
    Lock hlock;
    Lock vlock;
    Layout layout;

    // Cached client area (space allocated from parent)
    Viewport vp;

    // Cached absolute viewport (recomputed on resize - respects aspect)
    Viewport v;

    // Input event handler (if any)
    Handler* handler;

    // Map for sub-displays (if any)
    std::vector<View*> views;

  private:
    // Private copy constructor
    View(View& v) { /* Do Not copy - take reference instead*/ }
  };

  //! @brief Input Handler base class with virtual methods which recurse
  //! into sub-displays
  struct Handler
  {
    virtual void Keyboard(View&, unsigned char key, int x, int y);
    virtual void Mouse(View&, int button, int state, int x, int y);
    virtual void MouseMotion(View&, int x, int y);
  };
  static Handler StaticHandler;

  struct Handler3D : Handler
  {
    Handler3D(OpenGlRenderState& cam_state)
      : cam_state(&cam_state), hwin(3), tf(0.01), cameraspec(CameraSpecOpenGl) {};

    void SetOpenGlCamera();
    void Mouse(View&, int button, int state, int x, int y);
    void MouseMotion(View&, int x, int y);

    OpenGlRenderState* cam_state;
    int hwin;
    float tf;
    CameraSpec cameraspec;
    GLfloat last_z;
    GLint last_pos[2];
    GLdouble rot_center[3];
  };

  View& DisplayBase();
  View& Display(const std::string& name);

  OpenGlMatrixSpec ProjectionMatrix(int w, int h, double fu, double fv, double u0, double v0, double zNear, double zFar );
  OpenGlMatrixSpec IdentityMatrix(OpenGlStack type);

}

inline pangolin::Viewport::Viewport(GLint l,GLint b,GLint w,GLint h) : l(l),b(b),w(w),h(h) {};
inline GLint pangolin::Viewport::r() const { return l+w;}
inline GLint pangolin::Viewport::t() const { return b+h;}
inline GLfloat pangolin::Viewport::aspect() const { return (GLfloat)w / (GLfloat)h; }

#endif // PANGOLIN_DISPLAY_H

