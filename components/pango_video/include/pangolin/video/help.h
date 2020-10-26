#pragma once

#include <pangolin/utils/argagg.hpp>

#include <iostream>

namespace pangolin {

enum HelpVerbosity{
    SUMMARY = 0,
    SYNOPSIS,
    PARAMS
};

struct HelpParams {
    HelpVerbosity verbosity = SUMMARY;
    std::string registry;
    std::string scheme;
};

void Help( const HelpParams& params, std::ostream& ostream = std::cerr );

} // pangolin
