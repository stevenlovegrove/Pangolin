#include <pangolin/tools/video_viewer.h>

#include <pangolin/pangolin.h>
#include <pangolin/video/video_input.h>
#include <pangolin/gl/gltexturecache.h>
#include <pangolin/gl/glpixformat.h>
#include <pangolin/handler/handler_image.h>
#include <pangolin/utils/file_utils.h>
#include <pangolin/utils/timer.h>
#include <pangolin/display/image_view.h>
#include <pangolin/utils/sigstate.h>


namespace pangolin
{

void videoviewer_signal_quit(int) {
    pango_print_info("Caught signal. Program will exit after any IO is complete.\n");
    pangolin::QuitAll();
}

PANGOLIN_EXPORT
void RunVideoViewerUI(const std::string& input_uri, const std::string& output_uri)
{
    RegisterNewSigCallback(videoviewer_signal_quit, nullptr, SIGINT);
    RegisterNewSigCallback(videoviewer_signal_quit, nullptr, SIGTERM);

    /////////////////////////////////////////////////////////////////////////
    /// Register pangolin variables
    /////////////////////////////////////////////////////////////////////////

    pangolin::Var<int>  record_timelapse_frame_skip("viewer.record_timelapse_frame_skip", 1 );
    pangolin::Var<int>  end_frame("viewer.end_frame", std::numeric_limits<int>::max() );
    pangolin::Var<bool> video_wait("video.wait", true);
    pangolin::Var<bool> video_newest("video.newest", false);

    /////////////////////////////////////////////////////////////////////////
    /// Setup Video streams
    /////////////////////////////////////////////////////////////////////////

    // Open Video by URI
    pangolin::VideoInput video(input_uri, output_uri);
    const size_t num_streams = video.Streams().size();

    if(num_streams == 0) {
        pango_print_error("No video streams from device.\n");
        return;
    }

    // Output details of video stream
    for(size_t s = 0; s < num_streams; ++s) {
        const pangolin::StreamInfo& si = video.Streams()[s];
        std::cout << "Stream " << s << ": " << si.Width() << " x " << si.Height()
                  << " " << si.PixFormat().format << " (pitch: " << si.Pitch() << " bytes)" << std::endl;
    }

    // Check if video supports VideoPlaybackInterface
    pangolin::VideoPlaybackInterface* video_playback = pangolin::FindFirstMatchingVideoInterface<pangolin::VideoPlaybackInterface>(video);
    const int total_frames = video_playback ? video_playback->GetTotalFrames() : std::numeric_limits<int>::max();
    const int slider_size = (total_frames < std::numeric_limits<int>::max() ? 20 : 0);

    if( video_playback ) {
        if(total_frames < std::numeric_limits<int>::max() ) {
            std::cout << "Video length: " << total_frames << " frames" << std::endl;
        }
        end_frame = 0;
    }

    std::unique_ptr<unsigned char[]> buffer(new unsigned char[video.SizeBytes()+1]);

    /////////////////////////////////////////////////////////////////////////
    /// Create GUI
    /////////////////////////////////////////////////////////////////////////


    // Create OpenGL window - guess sensible dimensions
    pangolin::CreateWindowAndBind( "VideoViewer",
        (int)(video.Width() * num_streams),
        (int)(video.Height() + slider_size)
    );

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    pangolin::Var<int> frame("ui.frame", -1, 0, total_frames-1 );
    pangolin::Slider frame_slider("frame", frame.Ref() );

    if(video_playback && total_frames < std::numeric_limits<int>::max())
    {
        // frame_slider should be added first so that it can be rendered correctly.
        pangolin::DisplayBase().AddDisplay(frame_slider);
        frame_slider.SetBounds(0.0, pangolin::Attach::Pix(slider_size), 0.0, 1.0);
    }

    pangolin::View& container = pangolin::Display("streams");
    container.SetLayout(pangolin::LayoutEqual)
             .SetBounds(pangolin::Attach::Pix(slider_size), 1.0, 0.0, 1.0);

    std::vector<ImageView> stream_views(video.Streams().size());
    for(auto& sv : stream_views) {
        container.AddDisplay(sv);
    }

    std::vector<pangolin::Image<unsigned char> > images;

    /////////////////////////////////////////////////////////////////////////
    /// Register key shortcuts
    /////////////////////////////////////////////////////////////////////////


    const int FRAME_SKIP = 30;
    const char show_hide_keys[]  = {'1','2','3','4','5','6','7','8','9'};
    const char screenshot_keys[] = {'!','@','#','$','%','^','&','*','('};

    // Show/hide streams
    for(size_t v=0; v < container.NumChildren() && v < 9; v++) {
        pangolin::RegisterKeyPressCallback(show_hide_keys[v], [v,&container](){
            container[v].ToggleShow();
        } );
        pangolin::RegisterKeyPressCallback(screenshot_keys[v], [v,&images,&video](){
            if(v < images.size() && images[v].ptr) {
                try{
                    pangolin::SaveImage(
                        images[v], video.Streams()[v].PixFormat(),
                        pangolin::MakeUniqueFilename("capture.png")
                    );
                }catch(std::exception e){
                    pango_print_error("Unable to save frame: %s\n", e.what());
                }
            }
        } );
    }

    pangolin::RegisterKeyPressCallback('r', [&](){
        if(!video.IsRecording()) {
            video.SetTimelapse( static_cast<size_t>(record_timelapse_frame_skip) );
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
    pangolin::RegisterKeyPressCallback('w', [&](){
        video_wait = !video_wait;
        if(video_wait) {
            pango_print_info("Gui wait's for video frame.\n");
        }else{
            pango_print_info("Gui doesn't wait for video frame.\n");
        }
    });
    pangolin::RegisterKeyPressCallback('d', [&](){
        video_newest = !video_newest;
        if(video_newest) {
            pango_print_info("Discarding old frames.\n");
        }else{
            pango_print_info("Not discarding old frames.\n");
        }
    });
    pangolin::RegisterKeyPressCallback('<', [&](){
        if(video_playback) {
            frame = video_playback->Seek(frame - FRAME_SKIP) -1;
            end_frame = frame + 1;
        }else{
            pango_print_warn("Unable to skip backward.");
        }
    });
    pangolin::RegisterKeyPressCallback('>', [&](){
        if(video_playback) {
            frame = video_playback->Seek(frame + FRAME_SKIP) -1;
            end_frame = frame + 1;
        }else{
            end_frame = frame + FRAME_SKIP;
        }
    });
    pangolin::RegisterKeyPressCallback(',', [&](){
        if(video_playback) {
            frame = video_playback->Seek(frame - 1) -1;
            end_frame = frame+1;
        }else{
            pango_print_warn("Unable to skip backward.");
        }
    });
    pangolin::RegisterKeyPressCallback('.', [&](){
        // Pause at next frame
        end_frame = frame+1;
    });
    pangolin::RegisterKeyPressCallback('0', [&](){
        video.RecordOneFrame();
    });

    /////////////////////////////////////////////////////////////////////////
    /// Main GUI loop
    /////////////////////////////////////////////////////////////////////////

    video.Start();

    // Stream and display video
    while(!pangolin::ShouldQuit())
    {
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        glColor3f(1.0f, 1.0f, 1.0f);

        if(frame.GuiChanged()) {
            if(video_playback) {
                frame = video_playback->Seek(frame) -1;
            }
            end_frame = frame + 1;
        }

        if ( frame < end_frame ) {
            if( video.Grab(&buffer[0], images, video_wait, video_newest) ) {
                frame = frame +1;

                // Update images
                for(unsigned int i=0; i<images.size(); ++i) {
                    stream_views[i].SetImage(images[i], pangolin::GlPixFormat(video.Streams()[i].PixFormat() ));
                }
            }
        }

        // leave in pixel orthographic for slider to render.
        pangolin::DisplayBase().ActivatePixelOrthographic();
        if(video.IsRecording()) {
            pangolin::glRecordGraphic(pangolin::DisplayBase().v.w-14.0f, pangolin::DisplayBase().v.h-14.0f, 7.0f);
        }
        pangolin::FinishFrame();
    }
}

}
