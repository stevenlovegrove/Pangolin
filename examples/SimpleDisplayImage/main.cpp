#include <limits>
#include <iostream>
#include <pangolin/pangolin.h>

using namespace std;
using namespace pangolin;

void setImageData(float * imageArray, int width, int height){
  for(int i = 0 ; i < width*height;i++) {
    imageArray[i] = (float)rand()/RAND_MAX;
  }
}

int main( int /*argc*/, char* argv[] )
{
  // Create OpenGL window in single line thanks to GLUT
  CreateGlutWindowAndBind("Main",640,480);

  OpenGlRenderState s_cam;
  s_cam.Set(ProjectionMatrix(640,480,420,420,320,240,0.1,1000));
  s_cam.Set(IdentityMatrix(GlModelViewStack));

  View& d_cam = Display("cam")
      .SetAspect(-640/480.0)
      .SetHandler(new Handler3D(s_cam));

  View& d_image = Display("image")
      .SetBounds(1.0,0.3,20,0.3,640.0/480)
      .SetLock(LockLeft,LockTop);

  const int width =  640;
  const int height = 480;

  float * imageArray = new float[width*height];
  GlTexture imageTexture(width,height,GL_LUMINANCE32F_ARB);

  // Default hooks for exiting (Esc) and fullscreen (tab).
  while(!pangolin::ShouldQuit())
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    d_cam.Activate(s_cam);
    glEnable(GL_DEPTH_TEST);
    glColor3f(1.0,1.0,1.0);
    glutWireTeapot(10.0);

    //Set some random image data and upload to GPU
    setImageData(imageArray,width,height);
    imageTexture.Upload(imageArray,GL_LUMINANCE,GL_FLOAT);

    //display the image
    d_image.Activate();
    imageTexture.RenderToViewport();

    glutSwapBuffers();
    glutMainLoopEvent();
  }

  delete imageArray;

  return 0;
}
