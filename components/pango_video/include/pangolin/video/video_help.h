#pragma once

#include <iostream>
#include <pangolin/factory/factory_help.h>

namespace pangolin {

/// Print to \p out supported pixel format codes
/// \p color whether ANSI Color codes should be used for formatting
void PrintPixelFormats(std::ostream& out = std::cout, bool color = true);

/// Print to \p out general Video URL usage and registered VideoFactories
/// \p out the stream to stream the help message to
/// \p scheme_filter a constraint on schemes to print, or empty if all should be listed
/// \p level the level of detail to use when printing (see enum above)
void VideoHelp(
    std::ostream& out = std::cout, const std::string& scheme_filter="",
    HelpVerbosity verbosity = HelpVerbosity::SYNOPSIS
);

}
