#include <limits>
#include <iostream>
#include <pangolin/pangolin.h>
#include <pangolin/simple_math.h>

using namespace std;
using namespace pangolin;

void displayImage(float * imageArray, int width, int height){
  OpenGlRenderState::ApplyIdentity();

  GLuint  m_texname;
  glGenTextures(1, &m_texname);
  glBindTexture(GL_TEXTURE_2D, m_texname);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_FLOAT, imageArray);

  glEnable(GL_TEXTURE_2D);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glBegin(GL_QUADS);
  glTexCoord2f(0, 0);
  glVertex2d(-1,-1);
  glTexCoord2f(1, 0);
  glVertex2d(1,-1);
  glTexCoord2f(1, 1);
  glVertex2d(1,1);
  glTexCoord2f(0, 1);
  glVertex2d(-1,1);
  glEnd();

  glDisable(GL_TEXTURE_2D);
  glDeleteTextures(1,&m_texname);

}

void setImageData(float * imageArray, int width, int height){
  for(int i = 0 ; i < width*height;i++){
    imageArray[i] = (float)rand()/RAND_MAX;
  }
}

int main( int /*argc*/, char* argv[] )
{

  int width =  640;
  int height = 480;
  float * imageArray = new float[width*height];



  // Create OpenGL window in single line thanks to GLUT
  CreateGlutWindowAndBind("Main",640,480);

  OpenGlRenderState s_cam;
  s_cam.Set(ProjectionMatrix(640,480,420,420,320,240,0.1,1000));
  s_cam.Set(IdentityMatrix(GlModelView));

  // Define viewports with mixed fractional and pixel units
  //Display("panal").SetPos(10, 0, 0.0, 200);

  View& d_cam = Display("cam")
//      .SetAspect(640/480.0)
//      .SetLock(LockCenter,LockCenter)
      .SetHandler(new Handler3D(s_cam));

  View& d_image = Display("image")
      .SetBounds(1.0,0.5,20,0.5,640.0/480)
      .SetLock(LockLeft,LockTop);
  //.SetAspect(640/480.0);


  // Default hooks for exiting (Esc) and fullscreen (tab).
  while(!pangolin::ShouldQuit())
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



    int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
    int windowHeight =glutGet(GLUT_WINDOW_HEIGHT);
    s_cam.Set(ProjectionMatrix(
      windowWidth,windowHeight,640,640,windowWidth/2,windowHeight/2,0.1,1000
    ));

    // Activate efficiently by object
    d_cam.Activate(s_cam);
    glEnable(GL_DEPTH_TEST);


//    float windowAspect = glutGet(GLUT_WINDOW_WIDTH)/ (float)glutGet(GLUT_WINDOW_HEIGHT);
//    gluPerspective(80,windowAspect,0.1,1000);
    glColor3f(1.0,1.0,1.0);
    glutWireTeapot(10.0);

    //Set some random image data
    setImageData(imageArray,width,height);

    //display the image

    d_image.Activate();
    displayImage( imageArray,width,height);

    //Viewport(0,0,width/2,height/2).Activate();
    //displayImage( imageArray,width,height);


    glutSwapBuffers();
    glutMainLoopEvent();
  }

  delete imageArray;

  return 0;
}
