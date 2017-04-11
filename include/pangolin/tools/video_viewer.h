#include <pangolin/platform.h>
#include <pangolin/display/window.h>
#include <pangolin/video/video_input.h>

#include <mutex>
#include <thread>
#include <string>
#include <functional>

namespace pangolin
{

PANGOLIN_EXPORT
class VideoViewer
{
public:
    typedef std::function<void(const unsigned char* data,
                               const std::vector<Image<unsigned char> >& images,
                               const picojson::value& properties)> FrameChangedCallbackFn;

    static constexpr int FRAME_SKIP = 30;

    VideoViewer(const std::string& window_name, const std::string& input_uri, const std::string& output_uri = "video.pango" );
    VideoViewer(const VideoViewer&) = delete;

    ~VideoViewer();

    void Quit();

    inline int TotalFrames() const
    {
        return video_playback ? video_playback->GetTotalFrames() : std::numeric_limits<int>::max();
    }

    // Control playback
    void OpenInput(const std::string& input_uri);
    void CloseInput();

    // Control recording
    void Record();
    void RecordOneFrame();
    void StopRecording();

    // Useful for user-control
    void TogglePlay();
    void ToggleRecord();
    void ToggleDiscardBufferedFrames();
    void ToggleWaitForFrames();
    void SetDiscardBufferedFrames(bool new_state);
    void SetWaitForFrames(bool new_state);
    void Skip(int frames);

    // Register to be notified of new image data
    void SetFrameChangedCallback(FrameChangedCallbackFn cb);

    void WaitUntilExit();


    VideoInput& Video() {return video;}
    const VideoInput& Video() const {return video;}

protected:
    void RegisterDefaultKeyShortcutsAndPangoVariables();
    void Run();

    std::mutex control_mutex;
    std::string window_name;
    std::thread vv_thread;

    VideoInput video;
    VideoPlaybackInterface* video_playback;
    VideoPropertiesInterface* video_properties;

    std::string output_uri;

    int current_frame;
    int grab_until;
    int record_nth_frame;
    bool video_grab_wait;
    bool video_grab_newest;
    bool should_run;

    FrameChangedCallbackFn frame_changed_callback;
};


void PANGOLIN_EXPORT RunVideoViewerUI(const std::string& input_uri, const std::string& output_uri);

}
