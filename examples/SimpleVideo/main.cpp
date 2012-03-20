/**
 * @author  Steven Lovegrove
 * Copyright (C) 2010  Steven Lovegrove
 *                     Imperial College London
 **/

#include <pangolin/pangolin.h>
#include <pangolin/video.h>

using namespace pangolin;
using namespace std;

void VideoSample(const std::string uri)
{
    // Setup Video Source
    VideoInput video(uri);
    VideoPixelFormat vid_fmt = VideoFormatFromString(video.PixFormat());
    const unsigned w = video.Width();
    const unsigned h = video.Height();

    // Create Glut window
    pangolin::CreateGlutWindowAndBind("Main",w,h);

    // Create viewport for video with fixed aspect
    View& vVideo = Display("Video").SetAspect((float)w/h);

    // OpenGl Texture for video frame
    GlTexture texVideo(w,h,GL_RGBA8);

    unsigned char* img = new unsigned char[w*h*vid_fmt.size_bytes];

    for(int frame=0; !pangolin::ShouldQuit(); ++frame)
    {
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        video.GrabNext(img,true);
        texVideo.Upload(img, vid_fmt.channels==1 ? GL_LUMINANCE:GL_RGB, GL_UNSIGNED_BYTE);

        // Activate video viewport and render texture
        vVideo.Activate();
        texVideo.RenderToViewportFlipY();

        // Swap back buffer with front
        glutSwapBuffers();

        // Process window events via GLUT
        glutMainLoopEvent();
    }

    delete[] img;
}


int main( int argc, char* argv[] )
{
    std::string uri = "dc1394:[fps=30,dma=10,size=640x480,iso=400]//0";

    if( argc > 1 ) {
        uri = std::string(argv[1]);
    }else{
        cout << "Usage:" << endl << "\tSimpleRecord [video-uri]" << endl;
        cout << "\tvideo-uri: URI of file / device to extract video sequence from" << endl << endl;
        cout << "e.g." << endl;
        cout << "\tSimpleRecord dc1394:[fmt=RGB8,size=640x480,fps=30,iso=400,dma=10]//0" << endl;
        cout << "\tSimpleRecord dc1394:[fmt=FORMAT7_1,size=640x480,pos=2+2,iso=400,dma=10]//0" << endl;
        cout << "\tSimpleRecord convert:[fmt=RGB8]//v4l:///dev/video0" << endl;
        cout << "\tSimpleRecord file:///media/Data/pictures/photos/Minnesota 2010/00021.MTS" << endl;
        cout << "\tSimpleRecord file:///home/sl203/videos/YellowPattern1/test-0000000017.ppm" << endl;
        cout << "\tSimpleRecord file://http://192.168.0.150/?action=stream" << endl;
        cout << endl << "Defaulting to video-uri=" << uri << endl;
    }

    VideoSample(uri);
}
