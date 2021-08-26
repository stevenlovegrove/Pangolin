#include <pangolin/pangolin.h>
#include <pangolin/tools/video_viewer.h>
#include <pangolin/utils/argagg.hpp>
#include <pangolin/video/video_help.h>

int main( int argc, char* argv[] )
{
    const std::string dflt_output_uri = "pango:[unique_filename]//video.pango";

    argagg::parser argparser = {{
        { "help", {"-h", "--help"}, "shows this help message!", 0},
        { "scheme", {"-s", "--scheme"}, "filters the help message by scheme", 1},
        { "verbose", {"-v","--verbose"}, "verbose level in number, 0=list of schemes(default),1=scheme parameters,2=parameter details", 1}
    }};

    argagg::parser_results args = argparser.parse(argc, argv);
    if( args["help"] || args.pos.size() == 0){
        std::cerr << "Usage:\n";
        std::cerr << "  VideoViewer [options] VideoInputUri\n\n";
        std::cerr << "Examples:\n";
        std::cerr << "  VideoViewer test:[size=160x120,n=1,fmt=RGB24]//   Show the 'test' video driver with 160x120 resolution, 1 stream, RGB format.\n";
        std::cerr << "  VideoViewer --help -s image                       Find out how to use the 'image' video driver\n\n";
        std::cerr << "Options:\n";
        std::cerr << argparser << std::endl;

        const std::string scheme_filter = args["scheme"].as<std::string>("");
        const int v = std::clamp(args["verbose"].as<int>(scheme_filter.empty() ? 0 : 2), 0, 2);
        pangolin::VideoHelp(std::cerr, scheme_filter, (pangolin::HelpVerbosity)v );
        return 0;
    }

    const std::string input_uri = std::string(args.pos[0]);
    const std::string output_uri = (args.pos.size() > 1 ) ? std::string(args.pos[1]) : dflt_output_uri;
    try{
        pangolin::RunVideoViewerUI(input_uri, output_uri);
    } catch (const pangolin::VideoException& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
