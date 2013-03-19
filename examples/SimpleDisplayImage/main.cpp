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

  // 3D Mouse handler requires depth testing to be enabled
  glEnable(GL_DEPTH_TEST);
  
  pangolin::OpenGlRenderState s_cam(
    ProjectionMatrix(640,480,420,420,320,240,0.1,1000),
    ModelViewLookAt(-0,0.5,-3, 0,0,0, AxisY)
  );

  // Aspect ratio allows us to constrain width and height whilst fitting within specified
  // bounds. A positive aspect ratio makes a view 'shrink to fit' (introducing empty bars),
  // whilst a negative ratio makes the view 'grow to fit' (cropping the view).
  View& d_cam = Display("cam")
      .SetBounds(0,1,0,1,-640/480.0)
      .SetHandler(new Handler3D(s_cam));

  // This view will take up no more than a third of the windows width or height, and it
  // will have a fixed aspect ratio to match the image that it will display. When fitting
  // within the specified bounds, push to the top-left (as specified by SetLock).
  View& d_image = Display("image")
      .SetBounds(2/3.0,1.0,0,1/3.0,640.0/480)
      .SetLock(LockLeft,LockTop);

  cout << "Resize the window to experiment with SetBounds, SetLock and SetAspect." << endl;
  cout << "Notice that the teapots aspect is maintained even though it covers the whole screen." << endl;

  const int width =  640;
  const int height = 480;

  float* imageArray = new float[width*height];
  GlTexture imageTexture(width,height,GL_LUMINANCE32F_ARB);

  // Default hooks for exiting (Esc) and fullscreen (tab).
  while(!pangolin::ShouldQuit())
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    d_cam.Activate(s_cam);

    glColor3f(1.0,1.0,1.0);
    glutWireTeapot(2.0);

    //Set some random image data and upload to GPU
    setImageData(imageArray,width,height);
    imageTexture.Upload(imageArray,GL_LUMINANCE,GL_FLOAT);

    //display the image
    d_image.Activate();
    imageTexture.RenderToViewport();

    pangolin::FinishGlutFrame();
  }

  delete[] imageArray;

  return 0;
}
