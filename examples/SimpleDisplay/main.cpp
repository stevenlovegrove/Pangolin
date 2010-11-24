#include <iostream>
#include <pangolin/pangolin.h>

using namespace std;
using namespace pangolin;

int main( int /*argc*/, char* argv[] )
{
  // Create OpenGL window in single line thanks to GLUT
  CreateGlutWindowAndBind("Main",640,480);

  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  OpenGlRenderState s_cam;
  s_cam.Set(ProjectionMatrix(640,480,420,420,320,240,0.1,1000));
  s_cam.Set(IdentityMatrix(GlModelView));

  // Define viewports with mixed fractional and pixel units
  DisplayBase().views["panal"] = new Panal();
  View& d_panal = *DisplayBase().views["panal"];
  d_panal.SetBounds(1.0, 0.0, 0, 200);
  d_panal.layout = LayoutVertical;

//  View& d_panal = Display("panal")
//    .SetBounds(1.0, 0.0, 0, 200);
  View& d_cam = Display("cam")
//    .SetBounds(1.0, 0.0, 200, 1.0, 640.0f/480.0f)
    .SetAspect(640/480.0)
    .SetHandler(new Handler3D(s_cam));

  Button button1;
  Button button2;
  Button button3;
  d_panal.views["1"] = &button1;
  d_panal.views["2"] = &button2;
  d_panal.views["3"] = &button3;

  // Default hooks for exiting (Esc) and fullscreen (tab).
  while(!pangolin::ShouldQuit())
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Activate efficiently by object
    d_cam.Activate(s_cam);
    glEnable(GL_DEPTH_TEST);
    glColor3f(1.0,1.0,1.0);
    glutWireTeapot(10.0);

//    d_panal.Activate();
    OpenGlRenderState::ApplyWindowCoords();
    glDisable(GL_DEPTH_TEST);
    d_panal.Render();

    glutSwapBuffers();
    glutMainLoopEvent();
  }

  return 0;
}
