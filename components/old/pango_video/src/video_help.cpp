#include <pangolin/video/video_help.h>
#include <pangolin/video/video_interface.h>

#include <pangolin/factory/RegisterFactoriesVideoInterface.h>
#include <pangolin/factory/RegisterFactoriesVideoOutputInterface.h>

namespace pangolin
{

void PrintPixelFormats(std::ostream& out, bool color)
{
    const std::string c_normal = color ? "\033[0m"  : "";
    const std::string c_alias  = color ? "\033[32m" : "";

   out << "Supported pixel format codes (and their respective bits-per-pixel):" << std::endl;
   std::vector<pangolin::PixelFormat> pixelFormats = pangolin::GetSupportedPixelFormats();
   for(const auto& format: pixelFormats ){
       out << c_alias << format.format << c_normal << " (" << format.bpp << "), ";
   }
   out << std::endl;
}

void VideoHelp( std::ostream& out, const std::string& scheme_filter, HelpVerbosity verbosity)
{
    RegisterFactoriesVideoInterface();

#ifndef _WIN32_
    const bool use_color = true;
#else
    const bool use_color = false;
#endif

    if( verbosity >= HelpVerbosity::SYNOPSIS ) {
        PrintSchemeHelp(out, use_color);
        out << std::endl;
    }

    PrintFactoryRegistryDetails(out, *FactoryRegistry::I(), typeid(VideoInterface), scheme_filter, verbosity, use_color);

    if( verbosity >= HelpVerbosity::PARAMS ) {
        PrintPixelFormats(out, use_color);
    }
}

}
