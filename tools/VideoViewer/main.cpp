#include <pangolin/pangolin.h>
#include <pangolin/tools/video_viewer.h>
#include <pangolin/utils/argagg.hpp>
#include <pangolin/video/help.h>

int main( int argc, char* argv[] )
{
    const std::string dflt_output_uri = "pango:[unique_filename]//video.pango";

    argagg::parser argparser = {{
        { "help", {"-h", "--help"}, "shows this help! duh!", 0},
        { "registry", {"-r", "--registry"}, "filters the help message by registry", 1},
        { "scheme", {"-s", "--scheme"}, "filters the help message by scheme", 1},
        { "verbose", {"-v","--verbose"}, "verbose level in number, 0=list of schemes(default),1=scheme parameters,2=parameter details", 1}
    }};

    argagg::parser_results args = argparser.parse(argc, argv);
    if( args["help"]){
        std::cerr << "Generation options" << std::endl;
        std::cerr << argparser;
        std::cerr << std::endl;
        pangolin::HelpParams help_params;
        help_params.verbosity = pangolin::HelpVerbosity(std::min(std::max(args["verbose"].as<int>(0),0),2));
        help_params.registry = args["registry"].as<std::string>("");
        help_params.scheme = args["scheme"].as<std::string>("");
        pangolin::Help( help_params, std::cerr );
        return 0;
    }

    if( args.pos.size() > 0 ) {
        const std::string input_uri = std::string(args.pos[0]);
        const std::string output_uri = (args.pos.size() > 1 ) ? std::string(args.pos[1]) : dflt_output_uri;
        try{
            pangolin::RunVideoViewerUI(input_uri, output_uri);
        } catch (const pangolin::VideoException& e) {
            std::cout << e.what() << std::endl;
        }
    }else{
        const std::string input_uris[] = {
            "convert:[fmt=RGB24]//v4l:///dev/video0",
            "convert:[fmt=RGB24]//v4l:///dev/video1",
            "dc1394:[fps=30,dma=10,size=640x480,iso=400]//0",
            "openni:[img1=rgb]//",
            "test:[size=160x120,n=1,fmt=RGB24]//"
            ""
        };

        pangolin::Help( pangolin::HelpParams(), std::cerr );

        // Try to open some video device
        for(int i=0; !input_uris[i].empty(); ++i )
        {
            try{
                pango_print_info("Trying: %s\n", input_uris[i].c_str());
                pangolin::RunVideoViewerUI(input_uris[i], dflt_output_uri);
                return 0;
            }catch(const std::exception &e) {
              std::cout << e.what() << std::endl;
            }
        }
    }

    return 0;
}
