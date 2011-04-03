/**
 * @author  Steven Lovegrove
 * Copyright (C) 2010  Steven Lovegrove
 *                     Imperial College London
 **/

#include <GL/glew.h>

#include <boost/thread.hpp>
#include <pangolin/pangolin.h>
#include <pangolin/videosource.h>
#include <cvd/videosource.h>
#include <TooN/sl.h>
#include <TooN/so3.h>
#include <TooN/se3.h>
#include <TooN/LU.h>

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

using namespace pangolin;
using namespace std;
using namespace CVD;
using namespace TooN;

//#define USE_CVD_VID

int main( int /*argc*/, char* argv[] )
{
  // Setup Firewire Camera
  FirewireVideo video(0,DC1394_VIDEO_MODE_640x480_RGB8,DC1394_FRAMERATE_30,DC1394_ISO_SPEED_400,50);
  const unsigned w = video.Width();
  const unsigned h = video.Height();

  // Create Glut window
  pangolin::CreateGlutWindowAndBind("Main",w,h);

  // Create viewport for video with fixed aspect
  View& vVideo = Display("Video").SetAspect((float)w/h);

  // Video Image buffer
  GlTexture texVideo(w,h,GL_RGBA8);

  for(int frame=0; !pangolin::ShouldQuit(); ++frame)
  {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // Get newest frame from camera and upload to GPU as texture
    {
      FirewireFrame frame = video.GetNewest(true);
      texVideo.Upload(frame.Image(),GL_RGB,GL_UNSIGNED_BYTE);
      video.PutFrame(frame);
    }

    // Activate video viewport and render texture
    vVideo.Activate();
    texVideo.RenderToViewportFlipY();

    // Swap back buffer with front
    glutSwapBuffers();

    // Process window events via GLUT
    glutMainLoopEvent();
  }

  return 0;
}
