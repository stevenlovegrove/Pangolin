#include <iostream>
#include <pangolin/pangolin.h>

using namespace std;

int main( int /*argc*/, char* argv[] )
{
  pangolin::glut::CreateWindowAndBind("Main");

  while(!pangolin::ShouldQuit())
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glutSwapBuffers();
    glutMainLoopEvent();
  }

  return 0;
}
