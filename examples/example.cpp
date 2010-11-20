#include <iostream>
#include <GL/glut.h>
#include <GL/freeglut_ext.h>
#include <pangolin/pangolin.h>

using namespace std;

int main( int argc, char* argv[] )
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutCreateWindow("Sample Program");

  while(1)
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Some GL
    glutSwapBuffers();
    glutMainLoopEvent();
  }

  return 0;
}
