#include <iostream>
#include <pangolin/pangolin.h>
#include <pangolin/gl/gltexturecache.h>
#include <pangolin/utils/picojson.h>

const std::string INPUT_VIDEOS = "input_videos";
const std::string VIDEO_URI =    "uri";

struct InputVideo
{
    InputVideo(const std::string& name, const std::string& filename)
        : name(name), filename(filename), video(filename)
    {
        buffer.reset(new unsigned char[video.SizeBytes()]);
    }

    pangolin::Image<unsigned char> GetImage(size_t i) {
        return video.Streams()[i].StreamImage(buffer.get());
    }

    std::string name;
    std::string filename;
    std::unique_ptr<unsigned char[]> buffer;
    pangolin::VideoInput video;
};

struct Weights
{
    double Weight(const std::string& name) const {
        const auto i = weights.find(name);
        if(i == weights.end()) {
            return 0.0;
        }else{
            return i->second;
        }
    }

    void operator +=(const Weights& o)
    {
        for(const auto& p : o.weights)
        {
            weights[p.first] += p.second;
        }
    }

    void operator /=(double val)
    {
        for(auto& p : weights)
        {
            p.second /= val;
        }
    }

    std::map<std::string,double> weights;
};

struct Event : public Weights
{
    int frame;
};

inline bool operator<(const Event& lhs, const Event& rhs)
{
    return lhs.frame < rhs.frame;
}

struct Timeline
{
    Weights GetWeights(int frame)
    {
        Weights w;

        for(const Event& e : events) {
            if(e.frame > frame) break;

            w = e;
        }

        return w;
    }

    Weights GetSmoothWeights(int frame)
    {
        constexpr int dist = 10;
        Weights w;

        for(int i=-dist; i < dist; ++i) {
            w += GetWeights(frame + i);
        }

        w /= 2*dist;

        return w;
    }

    std::vector<Event> events;
};

Timeline& operator<<(Timeline& t, const pangolin::json::value& jsv)
{
    for(const auto& jse : jsv.get<pangolin::json::array>() )
    {
        Event event;
        event.frame = jse["frame"].get<int64_t>();
        if(jse.contains("show")) {
            const std::string video_name = jse["show"].to_str();
            event.weights[video_name] = 1.0;
        }
        t.events.push_back(event);
    }

    std::sort(t.events.begin(), t.events.end());

    return t;
}

int main(int argc, char* argv[])
{
    if(argc != 3) {
        std::cerr << "Expected one argument." << std::endl;
        return -1;
    }
    const std::string project_json_filename = argv[1];
    const std::string output_uri = argv[2];

    // Load JSON File
    pangolin::json::value json_project;
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
            const pangolin::json::value& js_video = kv.second;
            const std::string name = kv.first;
            const std::string uri = js_video[VIDEO_URI].to_str();
            std::cout << name << " = " << uri << std::endl;
            input_videos.emplace( std::make_pair(name, InputVideo(kv.first, uri) ) );
        }
    }

    // Parse timeline
    Timeline timeline;
    timeline << json_project["timeline"];

    // Create GUI
    pangolin::CreateWindowAndBind("Main",2160,1200);
    glEnable (GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(GL_ONE, GL_ONE);

    int frame = 0;
    // 20000*1024*4
    pangolin::DisplayBase().RecordOnRender(output_uri);

    while( !pangolin::ShouldQuit() )
    {
        // Pull from all videos
        bool grabbed_any = false;
        for(auto& vid : input_videos) {
            const bool success = vid.second.video.GrabNext(vid.second.buffer.get());
            grabbed_any = grabbed_any || success;
        }

        // Stop video if there are no streams left to grab from.
        if(!grabbed_any) {
            break;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        pangolin::DisplayBase().Activate();

        Weights weights = timeline.GetSmoothWeights(frame);

        // Composite with OpenGL
        for(auto& vid : input_videos) {
            for(size_t s=0; s < vid.second.video.Streams().size(); ++s) {
                const pangolin::StreamInfo& si = vid.second.video.Streams()[s];
                const pangolin::GlPixFormat fmt = pangolin::GlPixFormat(si.PixFormat());
                const size_t stride = (8*si.Pitch()) / si.PixFormat().bpp;
                pangolin::Image<unsigned char> image = vid.second.GetImage(s);
                pangolin::GlTexture& tex = pangolin::TextureCache::I().GlTex((GLsizei)image.w, (GLsizei)image.h, fmt.scalable_internal_format, fmt.glformat, GL_FLOAT);
                tex.Bind();
                glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)stride);
                tex.Upload(image.ptr,0,0, (GLsizei)image.w, (GLsizei)image.h, fmt.glformat, fmt.gltype);

                const float w = weights.Weight(vid.first);
                glColor3f(w, w, w);
                tex.RenderToViewportFlipY();
            }
        }

        pangolin::FinishFrame();
        ++frame;
    }

    return 0;
}
