/**
 * @author  Steven Lovegrove
 * Copyright (C) 2010  Steven Lovegrove
 *                     Imperial College London
 **/

#include <pangolin/pangolin.h>
#include <pangolin/video.h>
#include <pangolin/video_recorder.h>

using namespace pangolin;
using namespace std;

void RecordSample(const std::string uri, const std::string filename)
{
    // Setup Video Source
    VideoInput video(uri);
    const unsigned w = video.Width();
    const unsigned h = video.Height();

    // Setup async video recorder with 50 frame memory buffer
    VideoRecorder recorder(filename, w, h, "RGB24", w*h*3*50);

    // Create Glut window
    pangolin::CreateGlutWindowAndBind("Main",w,h);

    // Create viewport for video with fixed aspect
    View& vVideo = Display("Video").SetAspect((float)w/h);

    // OpenGl Texture for video frame
    GlTexture texVideo(w,h,GL_RGBA8);

    unsigned char* rgb = new unsigned char[video.Width()*video.Height()*3];

    for(int frame=0; !pangolin::ShouldQuit(); ++frame)
    {
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        video.GrabNext(rgb,true);
        texVideo.Upload(rgb,GL_RGB,GL_UNSIGNED_BYTE);

        // Record video frame
        recorder.RecordFrame(rgb);

        // Activate video viewport and render texture
        vVideo.Activate();
        texVideo.RenderToViewportFlipY();

        // Swap back buffer with front
        glutSwapBuffers();

        // Process window events via GLUT
        glutMainLoopEvent();
    }

    delete[] rgb;
}

int main( int argc, char* argv[] )
{
    std::string uri = "dc1394:[fps=30,dma=10,size=640x480,iso=400]//0";
    std::string filename = "video.pvn";

    if( argc >= 2 ) {
        uri = std::string(argv[1]);
        if( argc == 3 ) {
            filename = std::string(argv[2]);
        }
    }else{
        cout << "Usage:" << endl << "\tSimpleRecord video-uri filename" << endl;
        cout << "\tvideo-uri:\tURI of file / device to extract video sequence from" << endl;
        cout << "\tfilename:\tfilename to record pvn video in to" << endl << endl;
        cout << "e.g." << endl;
        cout << "\tSimpleRecord dc1394:[fps=30,dma=10,size=640x480,iso=400]//0 video.pvn" << endl;
        cout << "\tSimpleRecord v4l:///dev/video0 video.pvn" << endl;
        cout << endl << "Defaulting to video-uri=" << uri << endl;
    }

    RecordSample(uri, filename);
}
