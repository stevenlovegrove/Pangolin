#include <iostream>

#include <pangolin/factory/factory_registry.h>

namespace pangolin {

/// The level of detail to use when printing
enum HelpVerbosity{
    SUMMARY = 0, // Short description
    SYNOPSIS,    // + examples, aliases
    PARAMS       // + list all arguments
};

/// Print to \p out general guidance on how to use Pangolin factory URL's
/// \p out the stream to stream the help message to
/// \p color whether ANSI Color codes should be used for formatting
void PrintSchemeHelp(std::ostream& out = std::cout, bool color = true);

/// Print to \p out Factories registered to \p registry that match \p scheme_filter.
/// \p out the stream to stream the help message to
/// \p registry the registy to use
/// \p factory_type the typeid(T) of the FactoryInterface T to list
/// \p scheme_filter a constraint on schemes to print, or empty if all should be listed
/// \p level the level of detail to use when printing (see enum above)
/// \p color whether ANSI Color codes should be used for formatting
void PrintFactoryRegistryDetails(
    std::ostream& out, const pangolin::FactoryRegistry& registry, std::type_index factory_type,
    const std::string& scheme_filter = "", HelpVerbosity level = HelpVerbosity::SYNOPSIS, bool color = true
);

} 
