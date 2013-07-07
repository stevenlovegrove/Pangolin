#include <limits>
#include <iostream>
#include <pangolin/pangolin.h>
#include <pangolin/image_load.h>

using namespace std;
using namespace pangolin;

void setImageData(unsigned char * imageArray, int size){
  for(int i = 0 ; i < size;i++) {
    imageArray[i] = (unsigned char)(rand()/(RAND_MAX/255.0));
  }
}

int main( int /*argc*/, char* argv[] )
{
    std::string filename = "/Users/slovegrove/Documents/Drawing/Cool illustrations CFSL/31_32_clement_fait_pipi_est.jpg";
    pangolin::TypedImage img = pangolin::LoadImage(filename);    
    
  // Create OpenGL window in single line thanks to GLUT
  CreateWindowAndBind("Main",640,480);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // 3D Mouse handler requires depth testing to be enabled
  glEnable(GL_DEPTH_TEST);
  
  pangolin::OpenGlRenderState s_cam(
    ProjectionMatrix(640,480,420,420,320,240,0.1,1000),
    ModelViewLookAt(-1,1,-1, 0,0,0, AxisY)
  );

  // This view will take up no more than a third of the windows width or height, and it
  // will have a fixed aspect ratio to match the image that it will display. When fitting
  // within the specified bounds, push to the top-left (as specified by SetLock).
  View& d_image = Display("image")
      .SetAspect((float)img.w / (float)img.h)
      .SetLock(LockLeft,LockTop);

  cout << "Resize the window to experiment with SetBounds, SetLock and SetAspect." << endl;
  cout << "Notice that the teapots aspect is maintained even though it covers the whole screen." << endl;

//  const int width =  64;
//  const int height = 48;

//  unsigned char* imageArray = new unsigned char[3*width*height];
//  GlTexture imageTexture(width,height,GL_RGB,false,0,GL_RGB,GL_UNSIGNED_BYTE);

  GlTexture imageTexture(img.w, img.h, GL_RGB,false,0,GL_RGB,GL_UNSIGNED_BYTE);
  imageTexture.Upload(img.ptr, GL_RGB, GL_UNSIGNED_BYTE);
  
  // Default hooks for exiting (Esc) and fullscreen (tab).
  while(!pangolin::ShouldQuit())
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//    //Set some random image data and upload to GPU
//    setImageData(imageArray,3*width*height);
//    imageTexture.Upload(imageArray,GL_RGB,GL_UNSIGNED_BYTE);

    //display the image
    d_image.Activate();
    glColor3f(1.0,1.0,1.0);
    imageTexture.RenderToViewportFlipY();

    pangolin::FinishFrame();
  }

  img.Dealloc();
//  delete[] imageArray;

  return 0;
}
