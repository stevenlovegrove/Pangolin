/**
 * @author  Steven Lovegrove
 * Copyright (C) 2010  Steven Lovegrove
 *                     Imperial College London
 **/

#include <pangolin/pangolin.h>
#include <pangolin/video.h>
#include <pangolin/video_record_repeat.h>
#include <pangolin/input_record_repeat.h>

using namespace pangolin;
using namespace std;

void RecordSample(const std::string uri, const std::string vid_file, const std::string ui_file)
{
    // Setup Video Source
    VideoRecordRepeat video(uri, vid_file, 1024*1024*200);
    VideoPixelFormat vid_fmt = VideoFormatFromString(video.PixFormat());
    const unsigned w = video.Width();
    const unsigned h = video.Height();

    InputRecordRepeat input("ui.");
    input.LoadBuffer(ui_file);

    // Create Glut window
    const int panel_width = 200;
    pangolin::CreateGlutWindowAndBind("Main",w + panel_width,h);

    // Create viewport for video with fixed aspect
    View& d_panel = pangolin::CreatePanel("ui.")
        .SetBounds(1.0, 0.0, 0, panel_width);

    View& vVideo = Display("Video")
        .SetBounds(1.0, 0.0, panel_width, 1.0)
        .SetAspect((float)w/h);

    // OpenGl Texture for video frame
    GlTexture texVideo(w,h,GL_RGBA8);

    unsigned char* img = new unsigned char[w*h*vid_fmt.size_bytes];

    static Var<bool> record("ui.Record",false,false);
    static Var<bool> play("ui.Play",false,false);
    static Var<bool> source("ui.Source",false,false);
    static Var<bool> realtime("ui.realtime",true,true);

    static Var<float> hue("ui.Hue",0,0,360);
    static Var<bool> colour("ui.Colour Video",false,true);

    while( !pangolin::ShouldQuit() )
    {
        // Load next video frame
        video.GrabNext(img,true);
        texVideo.Upload(img, vid_fmt.channels==1 ? GL_LUMINANCE:GL_RGB, GL_UNSIGNED_BYTE);

        // Associate input with this video frame
        input.SetIndex(video.FrameId());

        // Activate video viewport and render texture
        vVideo.ActivateScissorAndClear();

        if( colour ) {
            glColorHSV(hue,0.5,1.0);
        }else{
            glColor3f(1,1,1);
        }
        texVideo.RenderToViewportFlipY();

        if(pangolin::Pushed(record)) {
            video.Record();
            input.Record();
        }

        if(pangolin::Pushed(play)) {
            video.Play(realtime);
            input.PlayBuffer(0,input.Size()-1);
            input.SaveBuffer(ui_file);
        }

        if(pangolin::Pushed(source)) {
            video.Source();
            input.Stop();
            input.SaveBuffer(ui_file);
        }

        d_panel.Render();

        glutSwapBuffers();
        glutMainLoopEvent();
    }

    delete[] img;
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
        cout << "Usage:" << endl << "\tSimpleRepeatVideo video-uri filename" << endl;
        cout << "\tvideo-uri:\tURI of file / device to extract video sequence from" << endl;
        cout << "\tfilename:\tfilename to record pvn video in to" << endl << endl;
        cout << "e.g." << endl;
        cout << "\tSimpleRepeatVideo dc1394:[fmt=RGB8,size=640x480,fps=30,iso=400,dma=10]//0 video.pvn" << endl;
        cout << "\tSimpleRepeatVideo dc1394:[fmt=FORMAT7_2,size=640x480,pos=2+2,iso=400,dma=10]//0" << endl;
        cout << "\tSimpleRepeatVideo convert:[fmt=RGB8]//v4l:///dev/video0 video.pvn" << endl;
        cout << endl << "Defaulting to video-uri=" << uri << endl;
    }

    RecordSample(uri, filename, filename + ".ui");
}
