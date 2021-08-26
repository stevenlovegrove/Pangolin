#include <pangolin/pangolin.h>
#include <pangolin/video/video_input.h>
#include <pangolin/factory/factory_registry.h>
#include <pangolin/utils/argagg.hpp>
#include <pangolin/image/pixel_format.h>
#include <pangolin/video/video_help.h>

void VideoConvert(const std::string& input_uri, const std::string& output_uri)
{
    pangolin::Var<bool> video_wait("video.wait", true);
    pangolin::Var<bool> video_newest("video.newest", false);

    // Open Video by URI
    pangolin::VideoInput video(input_uri, output_uri);
    const size_t num_streams = video.Streams().size();

    pangolin::VideoPlaybackInterface* playback = pangolin::FindFirstMatchingVideoInterface<pangolin::VideoPlaybackInterface>(video);

    // Output details of video stream
    for(size_t s = 0; s < num_streams; ++s)
    {
        const pangolin::StreamInfo& si = video.Streams()[s];
        std::cout << "Stream " << s << ": " << si.Width() << " x " << si.Height()
                  << " " << si.PixFormat().format << " (pitch: " << si.Pitch() << " bytes)" << std::endl;
    }

    // Image buffers
    std::vector<pangolin::Image<unsigned char> > images;
    std::vector<unsigned char> buffer;
    buffer.resize(video.SizeBytes()+1);

    // Record all frames
    video.Record();

    // Stream video
    while(true)
    {
        if( !video.Grab(&buffer[0], images, video_wait, video_newest) ) {
            break;
        }
        if( playback ) {
            std::cout << "Frames complete: " << playback->GetCurrentFrameId() << " / " << playback->GetTotalFrames() << '\r';
            std::cout.flush();
        }
   }
}

int main( int argc, char* argv[] )
{
    argagg::parser argparser = {{
        { "help", {"-h", "--help"}, "shows this help! duh!", 0},
        { "scheme", {"-s", "--scheme"}, "filters the help message by scheme", 1},
        { "verbose", {"-v","--verbose"}, "verbose level in number, 0=list of schemes(default),1=scheme parameters,2=parameter details", 1}
    }};

    argagg::parser_results args = argparser.parse(argc, argv);
    if( args["help"] || args.pos.size() == 0 ){
        std::cerr << "Usage:\n";
        std::cerr << "  VideoConvert [options] VideoInputUri\n\n";
        std::cerr << "Examples:\n";
        std::cerr << "  VideoConvert test:[size=160x120,n=1,fmt=RGB24]//   Show the 'test' video driver with 160x120 resolution, 1 stream, RGB format.\n";
        std::cerr << "  VideoConvert --help -s image                       Find out how to use the 'image' video driver\n\n";
        std::cerr << "Options:\n";
        std::cerr << argparser << std::endl;

        const std::string scheme_filter = args["scheme"].as<std::string>("");
        const int v = std::clamp(args["verbose"].as<int>(scheme_filter.empty() ? 0 : 2), 0, 2);
        pangolin::VideoHelp(std::cerr, scheme_filter, (pangolin::HelpVerbosity)v );
        return 0;
    }

    const std::string dflt_output_uri = "pango:[unique_filename]//video.pango";

    const std::string input_uri = std::string(args.pos[0]);
    const std::string output_uri = ( args.pos.size() > 1) ? std::string(args.pos[1]) : dflt_output_uri;
    try{
        VideoConvert(input_uri, output_uri);
    } catch (const pangolin::VideoException& e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
