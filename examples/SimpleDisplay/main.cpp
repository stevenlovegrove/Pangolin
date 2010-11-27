#include <iostream>
#include <pangolin/pangolin.h>

using namespace pangolin;
using namespace std;

int main( int /*argc*/, char* argv[] )
{
  // Create OpenGL window in single line thanks to GLUT
  pangolin::CreateGlutWindowAndBind("Main",640,480);

  // Issue specific OpenGl we might need
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Define Camera Render Object (for view / scene browsing)
  pangolin::OpenGlRenderState s_cam;
  s_cam.Set(ProjectionMatrix(640,480,420,420,320,240,0.1,1000));
  s_cam.Set(IdentityMatrix(GlModelView));

  // Add named OpenGL viewport to window and provide 3D Handler
  View& d_cam = pangolin::Display("cam")
    .SetBounds(1.0, 0.0, 200, 1.0, 640.0f/480.0f)
    .SetAspect(640/480.0)
    .SetHandler(new Handler3D(s_cam));

  // Add named Panal and bind to variables beginning 'ui'
  // A Panal is just a View with a default layout and input handling
  View& d_panal = pangolin::CreatePanal("panal","ui")
      .SetBounds(1.0, 0.0, 0, 200);

  // Default hooks for exiting (Esc) and fullscreen (tab).
  while(!pangolin::ShouldQuit())
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Safe and efficient binding of named variables.
    // Specialisations mean no conversions take place for exact types
    // and conversions between scalar types are cheap.
    static Var<bool> a_button("ui.A Button",false,false);
    static Var<double> a_double("ui.A Double",3,0,5.5);
    static Var<int> an_int("ui.An Int",2,0,5);
    static Var<bool> a_checkbox("ui.A Checkbox",false,true);

    if( Pushed(a_button) )
      cout << "You Pushed a button!" << endl;

    // Overloading of Var<T> operators allows us to treat them like
    // their wrapped types, eg:
    //   a_double = 1.32;
    //   an_int = a_double;
    //   a_checkbox = an_int > 2;

    // Activate efficiently by object
    // (3D Handler requires depth testing to be enabled)
    d_cam.Activate(s_cam);
    glEnable(GL_DEPTH_TEST);
    glColor3f(1.0,1.0,1.0);

    // Render some stuff
    glutWireTeapot(10.0);

    // Render our UI panal.
    d_panal.Render();

    // Swap frames and Process Events
    glutSwapBuffers();
    glutMainLoopEvent();
  }

  return 0;
}
