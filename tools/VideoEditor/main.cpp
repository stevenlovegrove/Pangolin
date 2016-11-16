#include <iostream>
#include <pangolin/pangolin.h>
#include <pangolin/utils/picojson.h>

const std::string INPUT_VIDEOS = "input_videos";

struct InputVideo
{
    InputVideo(const std::string& name, const std::string& filename)
        : name(name), filename(filename), video(filename)
    {

    }

    std::string name;
    std::string filename;
    pangolin::VideoInput video;
};

int main(int argc, char* argv[])
{
    if(argc != 2) {
        std::cerr << "Expected one argument." << std::endl;
        return -1;
    }

    // Load JSON File
    pangolin::json::value json_project;
    const std::string project_json_filename = argv[1];
    {
        std::ifstream f(project_json_filename);
        if(!f.is_open()) {
            std::cerr << "Could not open project file: '" << project_json_filename << "'" << std::endl;
            return -1;
        }
        const std::string json_load_error = pangolin::json::parse(json_project, f);
        if(!json_load_error.empty()) {
            std::cerr << json_load_error << std::endl;
            return -1;
        }
    }

    // Output project json for reference
    std::cout << json_project.serialize(true) << std::endl;

    // Instantiate referenced video objects
    std::map<std::string,InputVideo> input_videos;
    if(json_project.contains(INPUT_VIDEOS)) {
        pangolin::json::value js_videos = json_project[INPUT_VIDEOS];
        for(const auto& kv : js_videos.get<pangolin::json::object>() ) {
            input_videos.emplace( std::make_pair(kv.first, InputVideo(kv.first, kv.second.to_str())) );
            std::cout << kv.first << " = " << kv.second.to_str() << std::endl;
        }
    }

    // Create GUI
    pangolin::CreateWindowAndBind("Main",640,480);

    const int UI_WIDTH = 180;

    pangolin::CreatePanel("ui")
            .SetBounds(0.0, 1.0, 0.0, pangolin::Attach::Pix(UI_WIDTH));

    while( !pangolin::ShouldQuit() )
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        pangolin::FinishFrame();
    }

    return 0;
}
