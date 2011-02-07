#include <iostream>
#include <pangolin/pangolin.h>

using namespace pangolin;
using namespace std;

#include <pangolin/display_internal.h>

int main( int /*argc*/, char* argv[] )
{
  // Create OpenGL window in single line thanks to GLUT
  pangolin::CreateGlutWindowAndBind("Main",640,480);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Issue specific OpenGl we might need
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  DataLog log;

  View& d_graph = pangolin::CreatePlotter("x",log)
      .SetBounds(1.0, 0.0, 200, 1.0);

  View& d_panel = pangolin::CreatePanel("ui")
      .SetBounds(1.0, 0.0, 0, 200);

  double t = 0;

  // Default hooks for exiting (Esc) and fullscreen (tab).
  while(!pangolin::ShouldQuit())
  {
    if(HasResized())
      DisplayBase().ActivateScissorAndClear();

    static Var<double> tinc("ui.t inc",0.01,0,M_PI);

    d_graph.ActivateAndScissor();
    d_graph.Render();

    log.Log(sin(t),cos(t),tan(t));
    t += tinc;

    // Render our UI panel when we receive input
    if(HadInput())
      d_panel.Render();

    // Swap frames and Process Events
    glutSwapBuffers();
    glutMainLoopEvent();
  }

  return 0;
}
