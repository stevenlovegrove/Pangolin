#include <iostream>
#include <pangolin/pangolin.h>

using namespace std;
using namespace pangolin;

void doSomething()
{
  // Activate viewport by name
  pangolin::GetDisplay("cam")->Activate();
  glColor3f(1.0f, 0.0f, 0.0f);
  glRectf(-1,1,1,-1);
}

int main( int /*argc*/, char* argv[] )
{
  // Create OpenGL window in single line thanks to GLUT
  pangolin::glut::CreateWindowAndBind("Main",640,480);

  // Define viewports with mixed fractional and pixel units
  Display* d_panal = pangolin::AddDisplay("panal", 1.0, 0, 0.0, 200);

  // Default hooks for exiting (Esc) and fullscreen (tab).
  while(!pangolin::ShouldQuit())
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Can declare viewports anywhere (notice use of static)
    static Display* d_cam   = pangolin::AddDisplay("cam", 1.0, 200, 0.0, 1.0, 640.0f/480.0f);

    doSomething();

    // Activate efficiently by object
    d_panal->Activate();
    glColor3f(0.0f, 1.0f, 0.0f);
    glRectf(-1,1,1,-1);

    glutSwapBuffers();
    glutMainLoopEvent();
  }

  return 0;
}
