#include <pangolin/pangolin.h>
#include <pangolin/video/video_record_repeat.h>
#include <pangolin/gl/gltexturecache.h>
#include <pangolin/gl/glpixformat.h>

template<typename T>
std::pair<float,float> GetScaleBias(const pangolin::Image<unsigned char>& img)
{
    std::pair<float,float> mm(std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    for(size_t i=0; i < img.Area(); ++i) {
        const T val = ((T*)img.ptr)[i];
        if(val != 0) {
            if(val < mm.first) mm.first = val;
            if(val > mm.second) mm.second = val;
        }
    }

    const float scale = 1.0 / (mm.second - mm.first);
    const float bias = - scale*mm.first;
    return std::pair<float,float>(scale, bias);
}

template<typename T>
void ScaleImage(const pangolin::Image<unsigned char>& img, const std::pair<float,float>& scale_bias)
{
    for(size_t i=0; i < img.Area(); ++i) {
        T& val = ((T*)img.ptr)[i];
        val = val * scale_bias.first + scale_bias.second;
    }
}

void VideoViewer(const std::string& input_uri, const std::string& output_uri)
{
    // Open Video by URI
    pangolin::VideoRecordRepeat video(input_uri, output_uri);
    int total_frames = std::numeric_limits<int>::max();

    if(video.Streams().size() == 0) {
        pango_print_error("No video streams from device.\n");
        return;
    }

    // Check if video supports VideoPlaybackInterface
    pangolin::VideoPlaybackInterface* video_playback = video.Cast<pangolin::VideoPlaybackInterface>();
    if( video_playback ) {
        total_frames = video_playback->GetTotalFrames();
        std::cout << "Video length: " << total_frames << " frames" << std::endl;
    }

    std::vector<unsigned char> buffer;
    buffer.resize(video.SizeBytes()+1);

    // Create OpenGL window - guess sensible dimensions
    pangolin::CreateWindowAndBind( "VideoViewer",
        video.Width() * video.Streams().size(), video.Height()
    );

    // Setup resizable views for video streams
    std::vector<pangolin::GlPixFormat> glfmt;
    pangolin::DisplayBase().SetLayout(pangolin::LayoutEqual);
    for(unsigned int d=0; d < video.Streams().size(); ++d) {
        pangolin::View& view = pangolin::CreateDisplay().SetAspect(video.Streams()[d].Aspect());
        pangolin::DisplayBase().AddDisplay(view);
        glfmt.push_back(pangolin::GlPixFormat(video.Streams()[d].PixFormat()));
    }

    int frame = 0;
    pangolin::Var<int>  max_frame("max_frame", total_frames );
    pangolin::Var<bool> linear_sampling("linear_sampling", true );
    pangolin::Var<float> int16_scale("int16.scale", 1<<4 );
    pangolin::Var<float> int16_bias("int16.bias", 0.0 );
    pangolin::Var<float> float32_scale("float32.scale", 1.0 );
    pangolin::Var<float> float32_bias("float32.bias", 0.0 );

    std::vector<pangolin::Image<unsigned char> > images;

#ifdef CALLEE_HAS_CPP11
    const int FRAME_SKIP = 30;

    // Show/hide streams
    for(size_t v=0; v < pangolin::DisplayBase().NumChildren() && v < 9; v++) {
        pangolin::RegisterKeyPressCallback('1'+v, [v](){
            pangolin::DisplayBase()[v].ToggleShow();
        } );
    }

    pangolin::RegisterKeyPressCallback('r', [&](){
        if(!video.IsRecording()) {
            video.Record();
            pango_print_info("Started Recording.\n");
        }else{
            video.Stop();
            pango_print_info("Finished recording.\n");
        }
        fflush(stdout);
    });
    pangolin::RegisterKeyPressCallback('p', [&](){
        video.Play();
        max_frame = std::numeric_limits<int>::max();
        pango_print_info("Playing from file log.\n");
        fflush(stdout);
    });
    pangolin::RegisterKeyPressCallback('s', [&](){
        video.Source();
        max_frame = std::numeric_limits<int>::max();
        pango_print_info("Playing from source input.\n");
        fflush(stdout);
    });
    pangolin::RegisterKeyPressCallback(' ', [&](){
        max_frame = (frame < max_frame) ? frame : std::numeric_limits<int>::max();
    });
    pangolin::RegisterKeyPressCallback(pangolin::PANGO_SPECIAL + pangolin::PANGO_KEY_LEFT, [&](){
        if(video_playback) {
            const int frame = std::min(video_playback->GetCurrentFrameId()-FRAME_SKIP, video_playback->GetTotalFrames()-1);
            video_playback->Seek(frame);
        }else{
            // We can't go backwards
        }
    });
    pangolin::RegisterKeyPressCallback(pangolin::PANGO_SPECIAL + pangolin::PANGO_KEY_RIGHT, [&](){
        if(video_playback) {
            const int frame = std::max(video_playback->GetCurrentFrameId()+FRAME_SKIP, 0);
            video_playback->Seek(frame);
        }else{
            // Pause at this frame
            max_frame = frame+1;
        }
    });
    pangolin::RegisterKeyPressCallback('l', [&](){ linear_sampling = true; });
    pangolin::RegisterKeyPressCallback('n', [&](){ linear_sampling = false; });

    pangolin::RegisterKeyPressCallback('a', [&](){
        // Adapt scale
        for(unsigned int i=0; i<images.size(); ++i) {
            if(pangolin::DisplayBase()[i].IsShown()) {
                if(glfmt[i].gltype == GL_UNSIGNED_SHORT) {
                    std::pair<float,float> sb = GetScaleBias<unsigned short>(images[i]);
                    int16_scale = 255.0 * sb.first;
                    int16_bias  = 255.0 * sb.second;
                }else if(glfmt[i].gltype == GL_FLOAT) {
                    std::pair<float,float> sb = GetScaleBias<float>(images[i]);
                    float32_scale = sb.first;
                    float32_bias  = sb.second;
                }
            }
        }
    });
#endif


    // Stream and display video
    while(!pangolin::ShouldQuit())
    {
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        glColor3f(1.0f, 1.0f, 1.0f);

        if (frame == 0 || frame < max_frame) {
            images.clear();
            if (video.Grab(&buffer[0], images) ){
                ++frame;
            }
        }

        for(unsigned int i=0; i<images.size(); ++i)
        {
            if(pangolin::DisplayBase()[i].IsShown()) {
                pangolin::DisplayBase()[i].Activate();
                if(glfmt[i].gltype == GL_UNSIGNED_SHORT) {
                    pangolin::GlSlUtilities::Scale(int16_scale, int16_bias);
                    pangolin::RenderToViewport(images[i], glfmt[i], false, true, linear_sampling);
                    pangolin::GlSlUtilities::UseNone();
                }else if(glfmt[i].gltype == GL_FLOAT) {
                    pangolin::GlSlUtilities::Scale(float32_scale, float32_bias);
                    pangolin::RenderToViewport(images[i], glfmt[i], false, true, linear_sampling);
                    pangolin::GlSlUtilities::UseNone();
                }else{
                    pangolin::RenderToViewport(images[i], glfmt[i], false, true, linear_sampling);
                }
            }
        }

        pangolin::FinishFrame();
    }
}


int main( int argc, char* argv[] )
{
    const std::string dflt_output_uri = "pango://video.pango";

    if( argc > 1 ) {
        const std::string input_uri = std::string(argv[1]);
        const std::string output_uri = (argc > 2) ? std::string(argv[2]) : dflt_output_uri;
        try{
            VideoViewer(input_uri, output_uri);
        } catch (pangolin::VideoException e) {
            std::cout << e.what() << std::endl;
        }
    }else{
        const std::string input_uris[] = {
            "dc1394:[fps=30,dma=10,size=640x480,iso=400]//0",
            "convert:[fmt=RGB24]//v4l:///dev/video0",
            "convert:[fmt=RGB24]//v4l:///dev/video1",
            "openni:[img1=rgb]//",
            "test:[size=160x120,n=1,fmt=RGB24]//"
            ""
        };

        std::cout << "Usage  : VideoViewer [video-uri]" << std::endl << std::endl;
        std::cout << "Where video-uri describes a stream or file resource, e.g." << std::endl;
        std::cout << "\tfile:[realtime=1]///home/user/video/movie.pvn" << std::endl;
        std::cout << "\tfile:///home/user/video/movie.avi" << std::endl;
        std::cout << "\tfiles:///home/user/seqiemce/foo%03d.jpeg" << std::endl;
        std::cout << "\tdc1394:[fmt=RGB24,size=640x480,fps=30,iso=400,dma=10]//0" << std::endl;
        std::cout << "\tdc1394:[fmt=FORMAT7_1,size=640x480,pos=2+2,iso=400,dma=10]//0" << std::endl;
        std::cout << "\tv4l:///dev/video0" << std::endl;
        std::cout << "\tconvert:[fmt=RGB24]//v4l:///dev/video0" << std::endl;
        std::cout << "\tmjpeg://http://127.0.0.1/?action=stream" << std::endl;
        std::cout << "\topenni:[img1=rgb]//" << std::endl;
        std::cout << std::endl;

        // Try to open some video device
        for(int i=0; !input_uris[i].empty(); ++i )
        {
            try{
                pango_print_info("Trying: %s\n", input_uris[i].c_str());
                VideoViewer(input_uris[i], dflt_output_uri);
                return 0;
            }catch(pangolin::VideoException) { }
        }
    }

    return 0;
}
