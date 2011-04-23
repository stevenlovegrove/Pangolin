/**
 * @author  Steven Lovegrove
 * Copyright (C) 2010  Steven Lovegrove
 *                     Imperial College London
 **/

#include <pangolin/pangolin.h>
#include <pangolin/firewire.h>
#include <pangolin/v4l.h>
#include <pangolin/ffmpeg.h>

using namespace pangolin;

int firewire_sample()
{
    // Setup Firewire Camera
    FirewireVideo video(0,DC1394_VIDEO_MODE_640x480_RGB8,DC1394_FRAMERATE_30,DC1394_ISO_SPEED_400,50);
    const unsigned w = video.Width();
    const unsigned h = video.Height();

    // Create Glut window
    pangolin::CreateGlutWindowAndBind("Main",w,h);

    // Create viewport for video with fixed aspect
    View& vVideo = Display("Video").SetAspect((float)w/h);

    // OpenGl Texture for video frame
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

// borrowed tempararily and altered from libfreenect
// https://github.com/OpenKinect/libfreenect/blob/master/src/cameras.c
#define CLAMP(x) if (x < 0) {x = 0;} if (x > 255) {x = 255;}
static void convert_yuyv_to_rgb(uint8_t *raw_buf, uint8_t *proc_buf)
{
	int x, y;
	for(y = 0; y < 480; ++y) {
		for(x = 0; x < 640; x+=2) {
			int i = (640 * y + x);
			int y1 = raw_buf[2*i];
			int u  = raw_buf[2*i+1];
			int y2 = raw_buf[2*i+2];
			int v  = raw_buf[2*i+3];
			int r1 = (y1-16)*1164/1000 + (v-128)*1596/1000;
			int g1 = (y1-16)*1164/1000 - (v-128)*813/1000 - (u-128)*391/1000;
			int b1 = (y1-16)*1164/1000 + (u-128)*2018/1000;
			int r2 = (y2-16)*1164/1000 + (v-128)*1596/1000;
			int g2 = (y2-16)*1164/1000 - (v-128)*813/1000 - (u-128)*391/1000;
			int b2 = (y2-16)*1164/1000 + (u-128)*2018/1000;
			CLAMP(r1);
			CLAMP(g1);
			CLAMP(b1);
			CLAMP(r2);
			CLAMP(g2);
			CLAMP(b2);
			proc_buf[3*i]  =r1;
			proc_buf[3*i+1]=g1;
			proc_buf[3*i+2]=b1;
			proc_buf[3*i+3]=r2;
			proc_buf[3*i+4]=g2;
			proc_buf[3*i+5]=b2;
		}
	}
}
#undef CLAMP

// !V4L interface subject to change dramatically
int v4l_sample()
{
    // Setup Firewire Camera
    V4lVideo video("/dev/video0");
    //    FirewireVideo video(0,DC1394_VIDEO_MODE_640x480_RGB8,DC1394_FRAMERATE_30,DC1394_ISO_SPEED_400,50);
    const unsigned w = video.Width();
    const unsigned h = video.Height();

    // Create Glut window
    pangolin::CreateGlutWindowAndBind("Main",w,h);

    // Create viewport for video with fixed aspect
    View& vVideo = Display("Video").SetAspect((float)w/h);

    // OpenGl Texture for video frame
    GlTexture texVideo(w,h,GL_RGBA8);

    for(int frame=0; !pangolin::ShouldQuit(); ++frame)
    {
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        // Get newest frame from camera and upload to GPU as texture
        //        {
        //            FirewireFrame frame = video.GetNewest(true);
        //            texVideo.Upload(frame.Image(),GL_RGB,GL_UNSIGNED_BYTE);
        //            video.PutFrame(frame);
        //        }
        {
            static unsigned char yuv[640*480*3];
            static unsigned char rgb[640*480*3];
            video.GrabNext(yuv,true);
            convert_yuyv_to_rgb(yuv,rgb);
            texVideo.Upload(rgb,GL_RGB,GL_UNSIGNED_BYTE);
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

int ffmpeg_sample(const char* filename)
{
    // Setup Firewire Camera
    FfmpegVideo video(filename);
    const unsigned w = video.Width();
    const unsigned h = video.Height();

    // Create Glut window
    pangolin::CreateGlutWindowAndBind("Main",w,h);

    // Create viewport for video with fixed aspect
    View& vVideo = Display("Video").SetAspect((float)w/h);

    // OpenGl Texture for video frame
    GlTexture texVideo(w,h,GL_RGBA8);

    unsigned char* rgb = (unsigned char*)malloc(video.Width()*video.Height()*3);

    for(int frame=0; !pangolin::ShouldQuit(); ++frame)
    {
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        video.GrabNext(rgb,true);
        texVideo.Upload(rgb,GL_RGB,GL_UNSIGNED_BYTE);

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

void sample(const std::string uri)
{
    // Setup Firewire Camera
    VideoInput video(uri);
    const unsigned w = video.Width();
    const unsigned h = video.Height();

    // Create Glut window
    pangolin::CreateGlutWindowAndBind("Main",w,h);

    // Create viewport for video with fixed aspect
    View& vVideo = Display("Video").SetAspect((float)w/h);

    // OpenGl Texture for video frame
    GlTexture texVideo(w,h,GL_RGBA8);

    unsigned char* rgb = (unsigned char*)malloc(video.Width()*video.Height()*3);

    for(int frame=0; !pangolin::ShouldQuit(); ++frame)
    {
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        video.GrabNext(rgb,true);
        texVideo.Upload(rgb,GL_RGB,GL_UNSIGNED_BYTE);

        // Activate video viewport and render texture
        vVideo.Activate();
        texVideo.RenderToViewportFlipY();

        // Swap back buffer with front
        glutSwapBuffers();

        // Process window events via GLUT
        glutMainLoopEvent();
    }
}


int main( int argc, char* argv[] )
{
//    firewire_sample();
//    v4l_sample();
//    ffmpeg_sample("/media/Data/pictures/photos/Minnesota 2010/P1000142.jpg");
//    ffmpeg_sample("/media/Data/pictures/photos/Minnesota 2010/00021.MTS");
//    sample("file:///home/sl203/videos/YellowPattern1/test-0000000017.ppm");
    sample("v4l:///dev/video0");
//    sample("file:///media/Data/pictures/photos/Minnesota 2010/00021.MTS?something=67&test=something");
}
