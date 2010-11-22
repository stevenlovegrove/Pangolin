#include <iostream>
#include <pangolin/pangolin.h>
#include <pangolin/simple_math.h>

using namespace std;
using namespace pangolin;

void doSomething()
{
  // Activate viewport by name
  pangolin::GetDisplay("panal")->Activate();
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glColor3f(1.0f, 0.0f, 0.0f);
  glRectf(-1,1,1,-1);
}

int main( int /*argc*/, char* argv[] )
{
  // Create OpenGL window in single line thanks to GLUT
  pangolin::glut::CreateWindowAndBind("Main",640,480);

  OpenGlRenderState s_cam;
  s_cam.stacks[GlProjection] = ProjectionMatrix(640,480,420,420,320,240,0.1,1000);
  s_cam.stacks[GlModelView] = IdentityMatrix(GlModelView);

  // Define viewports with mixed fractional and pixel units
  Display* d_panal = pangolin::AddDisplay("panal", 1.0, 0, 0.0, 200);
  Display* d_cam   = pangolin::AddDisplay("cam", 1.0, 200, 0.0, 1.0, 640.0f/480.0f);
  d_cam->handler = new Handler3D(s_cam);

  // Default hooks for exiting (Esc) and fullscreen (tab).
  while(!pangolin::ShouldQuit())
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    doSomething();

    // Activate efficiently by object
    d_cam->Activate();
    s_cam.Apply();

    glutWireTeapot(10.0);

    glutSwapBuffers();
    glutMainLoopEvent();
  }

  return 0;
}
