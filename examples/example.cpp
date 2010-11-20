#include <iostream>
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/freeglut_ext.h>
#include <pangolin/pangolin.h>

using namespace std;

const unsigned char GLUT_KEY_ESCAPE = 27;
const unsigned char GLUT_KEY_TAB = 9;

bool run = true;

void glutkb( unsigned char key, int x, int y)
{
//  int mod = glutGetModifiers();

  if( key == GLUT_KEY_TAB)
    glutFullScreenToggle();
  else if( key == GLUT_KEY_ESCAPE)
    run = false;
  else if( key == ' ')
  {
    const int w = glutGet(GLUT_WINDOW_WIDTH);
    const int h = glutGet(GLUT_WINDOW_HEIGHT);
    cout << w << "x" << h << endl;
  }
}

int main( int argc, char* argv[] )
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutCreateWindow("Sample Program");
  glutKeyboardFunc(&glutkb);

  while(run)
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Some GL
    glutSwapBuffers();
    glutMainLoopEvent();
  }

  return 0;
}
