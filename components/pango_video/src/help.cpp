#include <pangolin/video/help.h>
#include <pangolin/video/video_drivers.h>
#include <pangolin/factory/factory_registry.h>
#include <pangolin/image/pixel_format.h>

namespace pangolin {
void PrintRegistryHelp( const pangolin::FactoryRegistryBase* registry, const std::string& scheme = "", HelpVerbosity level = HelpVerbosity::SUMMARY, std::ostream& output = std::cerr)
{
    output << "Registry: " << registry->GetType() << " (the number in the parenthesis before each scheme is its precedence order; lower values are of higher precedence)" << std::endl;
    for( const auto& f : registry->GetSchemeFactories() ){
        if( scheme.length() == 0 || !scheme.compare( f.scheme) ) {
            pangolin::FactoryHelpData help_data = f.Help();

            switch(level){
            case HelpVerbosity::SUMMARY:
                output << "\t" << "(" << f.precedence << ") " <<  f.scheme << (f.IsValidated() ? "" : " (WARNING: parameters are not validated!)") << std::endl;
                break;
            case HelpVerbosity::SYNOPSIS:
                output << "\t" << "(" << f.precedence << ") " << help_data.GetSynopsis() << (f.IsValidated() ? "" : " (WARNING: parameters are not validated!)") << std::endl;
                output << "\t     Description: " << help_data.GetDescription() << std::endl << std::endl;
                break;
            case HelpVerbosity::PARAMS:
                output << "\t" << "(" << f.precedence << ") " << help_data.GetSynopsis() << (f.IsValidated() ? "" : " (WARNING: parameters are not validated!)") << std::endl;
                output << "\t     Description: " << help_data.GetDescription() << std::endl;
                output << "\t     Parameters: (the assignment = denotes default value) " << std::endl;
                std::vector<pangolin::FactoryParamHelpData> param_help_data =  help_data.GetParamsHelp();
                for(const auto& param: param_help_data){
                    output << "\t\t- " << param.name << " = " << param.default_value <<": " << param.description << std::endl;
                }
                output << std::endl;
            }
        }
    }
}

void Help( const HelpParams& params, std::ostream& output )
{
    pangolin::LoadBuiltInVideoDrivers();

    output << "The scheme[s] are chosen from the following registries." << std::endl << std::endl;

    if( params.verbosity > HelpVerbosity::SUMMARY ){
        output << "Notation: " << std::endl;
        output << "\t* Parameter names are specified in regular expressions e.g. stream\\d+, which means the Uri can take multiple parameters such as stream0, steam1 etc." << std::endl;
        output << "\t* The assignment = for the parameter denotes default value." << std::endl;
        output << "\t* Asterisk * next to the default value denotes that the value is actually set dynamically during runtime based on other things." << std::endl;
        output << std::endl;
    }

    const std::vector<const pangolin::FactoryRegistryBase*> registry_list = pangolin::FactoryRegistryBase::GetFactoryRegistryList();

    for(const auto registry: registry_list ){
        const std::string& registryType = registry->GetType();
        if( params.registry.length() == 0 || !registryType.compare(params.registry) ){
            PrintRegistryHelp( registry, params.scheme, params.verbosity );
        }
        output << std::endl;
    }

    output << std::endl;

    output << "All supported image pixel formats:format(bit-per-pixel) " << std::endl;
    std::vector<pangolin::PixelFormat> pixelFormats = pangolin::GetSupportedPixelFormats();
    for(const auto& format: pixelFormats ){
        output << format.format << "(" << format.bpp << "), ";
    }

    output << std::endl;
}

}
