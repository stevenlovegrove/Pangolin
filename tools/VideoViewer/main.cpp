#include <pangolin/pangolin.h>
#include <pangolin/video/video_record_repeat.h>
#include <pangolin/gl/gltexturecache.h>
#include <pangolin/gl/glpixformat.h>

template<typename T>
std::pair<float,float> GetOffsetScale(const pangolin::Image<unsigned char>& img, float type_max, float format_max)
{
    std::pair<float,float> mm(std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    for(size_t i=0; i < img.Area(); ++i) {
        const T val = ((T*)img.ptr)[i];
        if(val != 0) {
            if(val < mm.first) mm.first = val;
            if(val > mm.second) mm.second = val;
        }
    }

    const float type_scale = format_max / type_max;
    const float offset = -type_scale* mm.first;
    const float scale = type_max / (mm.second - mm.first);
    return std::pair<float,float>(offset, scale);
}

void VideoViewer(const std::string& input_uri, const std::string& output_uri)
{
    int frame = 0;
    pangolin::Var<int>  end_frame("viewer.end_frame", std::numeric_limits<int>::max() );
    pangolin::Var<bool> linear_sampling("viewer.linear_sampling", true );

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
        if(total_frames < std::numeric_limits<int>::max() ) {
            std::cout << "Video length: " << total_frames << " frames" << std::endl;
            end_frame = 1;
        }
    }

    std::vector<unsigned char> buffer;
    buffer.resize(video.SizeBytes()+1);

    // Create OpenGL window - guess sensible dimensions
    pangolin::CreateWindowAndBind( "VideoViewer",
        video.Width() * video.Streams().size(), video.Height()
    );

    // Assume packed OpenGL data unless otherwise specified
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // Setup resizable views for video streams
    std::vector<pangolin::GlPixFormat> glfmt;
    std::vector<std::pair<float,float> > gloffsetscale;

    pangolin::View& container = pangolin::Display("streams");
    container.SetLayout(pangolin::LayoutEqual);
    for(unsigned int d=0; d < video.Streams().size(); ++d) {
        pangolin::View& view = pangolin::CreateDisplay().SetAspect(video.Streams()[d].Aspect());
        container.AddDisplay(view);
        glfmt.push_back(pangolin::GlPixFormat(video.Streams()[d].PixFormat()));
        gloffsetscale.push_back(std::pair<float,float>(0.0f, 1.0f) );
    }

    std::vector<pangolin::Image<unsigned char> > images;

#ifdef CALLEE_HAS_CPP11
    const int FRAME_SKIP = 30;
    const char show_hide_keys[]  = {'1','2','3','4','5','6','7','8','9'};
    const char screenshot_keys[] = {'!','"','#','$','%','^','&','*','('};

    // Show/hide streams
    for(size_t v=0; v < container.NumChildren() && v < 9; v++) {
        pangolin::RegisterKeyPressCallback(show_hide_keys[v], [v,&container](){
            container[v].ToggleShow();
        } );
        pangolin::RegisterKeyPressCallback(screenshot_keys[v], [v,&images,&video](){
            if(v < images.size() && images[v].ptr) {
                try{
                    pangolin::SaveImage(images[v],video.Streams()[v].PixFormat(), "still.png");
                }catch(std::exception e){
                    pango_print_error("Unable to save frame: %s\n", e.what());
                }
            }
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
        end_frame = std::numeric_limits<int>::max();
        pango_print_info("Playing from file log.\n");
        fflush(stdout);
    });
    pangolin::RegisterKeyPressCallback('s', [&](){
        video.Source();
        end_frame = std::numeric_limits<int>::max();
        pango_print_info("Playing from source input.\n");
        fflush(stdout);
    });
    pangolin::RegisterKeyPressCallback(' ', [&](){
        end_frame = (frame < end_frame) ? frame : std::numeric_limits<int>::max();
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
            end_frame = frame+1;
        }
    });
    pangolin::RegisterKeyPressCallback('l', [&](){ linear_sampling = true; });
    pangolin::RegisterKeyPressCallback('n', [&](){ linear_sampling = false; });

    pangolin::RegisterKeyPressCallback('a', [&](){
        // Adapt scale
        for(unsigned int i=0; i<images.size(); ++i) {
            if(container[i].HasFocus()) {
                std::pair<float,float> os(0.0f, 1.0f);
                if(glfmt[i].gltype == GL_UNSIGNED_BYTE) {
                    os = GetOffsetScale<unsigned char>(images[i], 255.0f, 1.0f);
                }else if(glfmt[i].gltype == GL_UNSIGNED_SHORT) {
                    os = GetOffsetScale<unsigned short>(images[i], 65535.0f, 1.0f);
                }else if(glfmt[i].gltype == GL_FLOAT) {
                    os = GetOffsetScale<float>(images[i], 1.0f, 1.0f);
                }
                gloffsetscale[i] = os;
            }
        }
    });
#endif


    // Stream and display video
    while(!pangolin::ShouldQuit())
    {
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        glColor3f(1.0f, 1.0f, 1.0f);

        if (frame == 0 || frame < end_frame) {
            if (video.Grab(&buffer[0], images) ){
                ++frame;
            }
        }

        for(unsigned int i=0; i<images.size(); ++i)
        {
            if(container[i].IsShown()) {
                container[i].Activate();
                const std::pair<float,float> os = gloffsetscale[i];
                pangolin::GlSlUtilities::OffsetAndScale(os.first, os.second);
                pangolin::RenderToViewport(images[i], glfmt[i], false, true, linear_sampling);
                pangolin::GlSlUtilities::UseNone();
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
