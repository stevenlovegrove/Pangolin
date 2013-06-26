#include <iostream>
#include <pangolin/pangolin.h>

using namespace pangolin;
using namespace std;

void setImageData(unsigned char * imageArray, int size){
  for(int i = 0 ; i < size;i++) {
    imageArray[i] = (unsigned char)(rand()/(RAND_MAX/255.0));
  }
}

int main( int /*argc*/, char* argv[] )
{
  // Create OpenGL window in single line thanks to GLUT
  pangolin::CreateWindowAndBind("Main",640,480);
  
  // 3D Mouse handler requires depth testing to be enabled
  glEnable(GL_DEPTH_TEST);  

  // Issue specific OpenGl we might need
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Define Camera Render Object (for view / scene browsing)
  pangolin::OpenGlMatrix proj = ProjectionMatrix(640,480,420,420,320,240,0.1,1000);
  pangolin::OpenGlRenderState s_cam(proj,ModelViewLookAt(1,0.5,-2,0,0,0,AxisY) );
  pangolin::OpenGlRenderState s_cam2(proj,ModelViewLookAt(0,0,-2,0,0,0,AxisY) );

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
  unsigned char* imageArray = new unsigned char[3*width*height];
  GlTexture imageTexture(width,height,GL_RGB,false,0,GL_RGB,GL_UNSIGNED_BYTE);

  // Default hooks for exiting (Esc) and fullscreen (tab).
  while( !pangolin::ShouldQuit() )
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Generate random image and place in texture memory for display
    setImageData(imageArray,3*width*height);
    imageTexture.Upload(imageArray,GL_RGB,GL_UNSIGNED_BYTE);

    glColor3f(1.0,1.0,1.0);

    d_cam1.Activate(s_cam);
    glDrawColouredCube();

    d_cam2.Activate(s_cam2);
    glDrawColouredCube();

    d_cam3.Activate(s_cam);
    glDrawColouredCube();

    d_cam4.Activate(s_cam2);
    glDrawColouredCube();

    d_img1.Activate();
    glColor4f(1.0f,1.0f,1.0f,1.0f);
    imageTexture.RenderToViewport();

    d_img2.Activate();
    glColor4f(1.0f,1.0f,1.0f,1.0f);
    imageTexture.RenderToViewport();

    // Swap frames and Process Events
    pangolin::FinishFrame();
  }

  delete[] imageArray;

  return 0;
}
