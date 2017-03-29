#include <pangolin/platform.h>
#include <pangolin/display/window.h>
#include <pangolin/video/video_input.h>

#include <mutex>
#include <thread>
#include <string>

namespace pangolin
{

PANGOLIN_EXPORT
class VideoViewer
{
public:
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
    void Skip(int frames);

    void WaitUntilExit();

protected:
    void RegisterDefaultKeyShortcutsAndPangoVariables();
    void Run();

    std::mutex control_mutex;
    std::string window_name;
    std::thread vv_thread;

    pangolin::VideoInput video;
    pangolin::VideoPlaybackInterface* video_playback;

    std::string output_uri;

    int current_frame;
    int grab_until;
    int record_nth_frame;
    bool video_grab_wait;
    bool video_grab_newest;
    bool should_run;
};


void PANGOLIN_EXPORT RunVideoViewerUI(const std::string& input_uri, const std::string& output_uri);

}
