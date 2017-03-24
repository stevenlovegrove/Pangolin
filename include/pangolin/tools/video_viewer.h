#include <pangolin/platform.h>
#include <pangolin/video/video_input.h>
#include <pangolin/var/var.h>

#include <mutex>
#include <thread>
#include <string>
#include <atomic>

namespace pangolin
{


class VideoViewer
{
public:
    
    VideoViewer(const std::string& input_uri, const std::string& output_uri, const bool new_thread=false);
    ~VideoViewer();
    
    void StartRecording();
    void StopRecording();
    void RecordOneFrame();
    void ChangeOutputURL(const std::string& url);
    
    pangolin::Var<bool>& VideoWait();
    pangolin::Var<bool>& VideoNewest();
    
private:
    
    void RunUI();
    
    VideoInput video_;
    
    pangolin::Var<bool> video_wait_;
    pangolin::Var<bool> video_newest_;
    
    
    std::mutex mutex;
    std::unique_ptr<std::thread> thread_;
    std::atomic<bool> toquit_;
    
    
};



void RunVideoViewerUI(const std::string& input_uri, const std::string& output_uri);

}
