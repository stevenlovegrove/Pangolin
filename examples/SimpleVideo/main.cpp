/**
 * @author  Steven Lovegrove
 * Copyright (C) 2010  Steven Lovegrove
 *                     Imperial College London
 **/

#include <pangolin/pangolin.h>
#include <pangolin/video.h>

using namespace pangolin;
using namespace std;

void SetGlFormat(GLint& glchannels, GLenum& glformat, const VideoPixelFormat& fmt)
{
    switch( fmt.channels) {
    case 1: glchannels = GL_LUMINANCE; break;
    case 3: glchannels = GL_RGB; break;
    case 4: glchannels = GL_RGBA; break;
    default: throw std::runtime_error("Unable to display video format");
    }

    switch (fmt.channel_bits[0]) {
    case 8: glformat = GL_UNSIGNED_BYTE; break;
    case 16: glformat = GL_UNSIGNED_SHORT; break;
    case 32: glformat = GL_FLOAT; break;
    default: throw std::runtime_error("Unknown channel format");
    }
}

void VideoSample(const std::string uri)
{
    // Setup Video Source
    VideoInput video(uri);
    const VideoPixelFormat vid_fmt = video.PixFormat();
    const unsigned w = video.Width();
    const unsigned h = video.Height();
#if !defined(HAVE_GLES) || defined(HAVE_GLES_2)
    const float scale = video.VideoUri().Get<float>("scale", 1.0f);
    const float bias  = video.VideoUri().Get<float>("bias", 0.0f);
#endif

    // Work out appropriate GL channel and format options
    GLint glchannels;
    GLenum glformat;
    SetGlFormat(glchannels, glformat, vid_fmt);
    
    // Create Glut window
    pangolin::CreateWindowAndBind("Main",w,h);

    // Create viewport for video with fixed aspect
    View& vVideo = Display("Video").SetAspect((float)w/h);

    // OpenGl Texture for video frame.
    GlTexture texVideo(w,h,glchannels,false,0,glchannels,glformat);

    unsigned char* img = new unsigned char[video.SizeBytes()];

    for(int frame=0; !pangolin::ShouldQuit(); ++frame)
    {
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        if( video.GrabNext(img,true) ) {
            texVideo.Upload( img, glchannels, glformat );
        }

        // Activate video viewport and render texture
        vVideo.Activate();
#if !defined(HAVE_GLES) || defined(HAVE_GLES_2)
        pangolin::GlSlUtilities::Scale(scale, bias);
        texVideo.RenderToViewportFlipY();
        pangolin::GlSlUtilities::UseNone();
#else
        texVideo.RenderToViewportFlipY();
#endif

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
        "openni:[img1=rgb]//",
        "test:[size=160x120,n=1,fmt=RGB24]//"
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
            }catch(VideoException) { }
        }
    }

    return 0;
}
