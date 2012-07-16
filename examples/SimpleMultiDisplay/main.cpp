#include <iostream>
#include <pangolin/pangolin.h>

using namespace pangolin;
using namespace std;

void setImageData(float * imageArray, int width, int height){
  for(int i = 0 ; i < width*height;i++) {
    imageArray[i] = (float)rand()/RAND_MAX;
  }
}

int main( int /*argc*/, char* argv[] )
{
  // Create OpenGL window in single line thanks to GLUT
  pangolin::CreateGlutWindowAndBind("Main",640,480);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Issue specific OpenGl we might need
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Define Camera Render Object (for view / scene browsing)
  pangolin::OpenGlMatrix proj = ProjectionMatrix(640,480,420,420,320,240,0.1,1000);
  pangolin::OpenGlRenderState s_cam(proj);
  pangolin::OpenGlRenderState s_cam2(proj);

  // Add named OpenGL viewport to window and provide 3D Handler
  View& d_cam1 = pangolin::Display("cam1")
    .SetAspect(640.0f/480.0f)
    .SetHandler(new Handler3D(s_cam));

  View& d_cam2 = pangolin::Display("cam2")
    .SetAspect(640.0f/480.0f)
    .SetHandler(new Handler3D(s_cam2));

  View& d_cam3 = pangolin::Display("cam3")
    .SetAspect(640.0f/480.0f)
    .SetHandler(new Handler3D(s_cam));

  View& d_cam4 = pangolin::Display("cam4")
    .SetAspect(640.0f/480.0f)
    .SetHandler(new Handler3D(s_cam2));

  View& d_img1 = pangolin::Display("img1")
    .SetAspect(640.0f/480.0f);

  View& d_img2 = pangolin::Display("img2")
    .SetAspect(640.0f/480.0f);

  // LayoutEqual is an EXPERIMENTAL feature - it requires that all sub-displays
  // share the same aspect ratio, placing them in a raster fasion in the
  // viewport so as to maximise display size.
  pangolin::Display("multi")
      .SetBounds(1.0, 0.0, 0.0, 1.0)
      .SetLayout(LayoutEqual)
      .AddDisplay(d_cam1)
      .AddDisplay(d_img1)
      .AddDisplay(d_cam2)
      .AddDisplay(d_img2)
      .AddDisplay(d_cam3)
      .AddDisplay(d_cam4);

  const int width =  64;
  const int height = 48;
  float * imageArray = new float[width*height];
  GlTexture imageTexture(width,height,GL_LUMINANCE32F_ARB);
  imageTexture.SetNearestNeighbour();

  // Default hooks for exiting (Esc) and fullscreen (tab).
  while( !pangolin::ShouldQuit() )
  {
    if(HasResized())
      DisplayBase().ActivateScissorAndClear();

    // Generate random image and place in texture memory for display
    setImageData(imageArray,width,height);
    imageTexture.Upload(imageArray,GL_LUMINANCE,GL_FLOAT);

    // (3D Handler requires depth testing to be enabled)
    glEnable(GL_DEPTH_TEST);
    glColor3f(1.0,1.0,1.0);

    d_cam1.ActivateScissorAndClear(s_cam);
    glutWireTeapot(1.0);

    d_cam2.ActivateScissorAndClear(s_cam2);
    glutWireTeapot(1.0);

    d_cam3.ActivateScissorAndClear(s_cam);
    glutWireTeapot(1.0);

    d_cam4.ActivateScissorAndClear(s_cam2);
    glutWireTeapot(1.0);

    d_img1.ActivateScissorAndClear();
    imageTexture.RenderToViewport();

    d_img2.ActivateScissorAndClear();
    imageTexture.RenderToViewport();

    // Swap frames and Process Events
    pangolin::FinishGlutFrame();
  }

  delete imageArray;

  return 0;
}
