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
    const VideoPixelFormat vid_fmt = video.PixFormat();
    const unsigned w = video.Width();
    const unsigned h = video.Height();

    // Create Glut window
    pangolin::CreateWindowAndBind("Main",w,h);

    // Create viewport for video with fixed aspect
    View& vVideo = Display("Video").SetAspect((float)w/h);

    // OpenGl Texture for video frame.
    // GL_RGBA8 is the internal_format, if we upload another it'll be converted
    GlTexture texVideo(w,h,GL_RGBA8);

    unsigned char* img = new unsigned char[video.SizeBytes()];

    for(int frame=0; !pangolin::ShouldQuit(); ++frame)
    {
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        if( video.GrabNext(img,true) ) {
            // TODO: Work out the correct display format properly
            texVideo.Upload(
                img, vid_fmt.channels==1 ? GL_LUMINANCE:GL_RGB,
                vid_fmt.channel_bits[0]==8?GL_UNSIGNED_BYTE:GL_UNSIGNED_SHORT
            );
        }

        // Activate video viewport and render texture
        vVideo.Activate();
        texVideo.RenderToViewportFlipY();

        // Swap back buffer with front and process window events via GLUT
        pangolin::FinishFrame();
    }

    delete[] img;
}


int main( int argc, char* argv[] )
{
    std::string uris[] = {
        "dc1394:[fps=30,dma=10,size=640x480,iso=400]//0",
        "convert:[fmt=RGB24]//v4l:///dev/video0",
        "convert:[fmt=RGB24]//v4l:///dev/video1",
        "openni:[img1=rgb]//"
        ""
    };

    if( argc > 1 ) {
        const string uri = std::string(argv[1]);
        VideoSample(uri);
    }else{
        cout << "Usage  : SimpleRecord [video-uri]" << endl << endl;
        cout << "Where video-uri describes a stream or file resource, e.g." << endl;
        cout << "\tfile:[realtime=1]///home/user/video/movie.pvn" << endl;
        cout << "\tfile:///home/user/video/movie.avi" << endl;
        cout << "\tfiles:///home/user/seqiemce/foo%03d.jpeg" << endl;
        cout << "\tdc1394:[fmt=RGB24,size=640x480,fps=30,iso=400,dma=10]//0" << endl;
        cout << "\tdc1394:[fmt=FORMAT7_1,size=640x480,pos=2+2,iso=400,dma=10]//0" << endl;
        cout << "\tv4l:///dev/video0" << endl;
        cout << "\tconvert:[fmt=RGB24]//v4l:///dev/video0" << endl;
        cout << "\tmjpeg://http://127.0.0.1/?action=stream" << endl;
        cout << "\topenni:[img1=rgb]//" << endl;
        cout << endl;

        // Try to open some video device
        for(int i=0; !uris[i].empty(); ++i )
        {
            try{
                cout << "Trying: " << uris[i] << endl;
                VideoSample(uris[i]);
                return 0;
            }catch(VideoException) {}
        }
    }

    return 0;
}
