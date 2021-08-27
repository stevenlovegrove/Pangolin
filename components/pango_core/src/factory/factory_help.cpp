#include <iomanip>
#include <numeric>
#include <limits>
#include <pangolin/factory/factory_help.h>
#include <pangolin/factory/factory_registry.h>

namespace pangolin {

void PrintSchemeHelp(std::ostream& out, bool color)
{
    const std::string c_normal = color ? "\033[0m"  : "";
    const std::string c_bold   = color ? "\033[1m"  : "";
    const std::string c_scheme = color ? "\033[36m" : "";
    const std::string c_alias  = color ? "\033[32m" : "";
    const std::string c_params = color ? "\033[34m" : "";
    const std::string c_param  = color ? "\033[31m" : "";

    out << c_bold << "Using Factory Schemes" << c_normal << std::endl << std::endl;

    out << "The factory to use is specified in the 'scheme' portion of the URL. Any parameters the factory takes will be reset to default values." << std::endl;
    out << " e.g. \"" << c_scheme << "scheme" << c_normal << "://" << c_normal << "\"" << std::endl;
    out << std::endl;

    out << "When a factory needs to choose a unique resource, it can be specified using the URI portion of the URL." << std::endl;
    out << " e.g. \"" << c_scheme << "scheme" << c_normal << "://" << c_param << "uri" << c_normal << "\"" << std::endl;
    out << std::endl;

    out << "Parameters can be specified within square brackets to override defaults." << std::endl;
    out << " e.g. \"" << c_scheme << "scheme" << c_normal << ":[";
    out << c_param << "param1" << c_normal << "=" << c_alias << "value1" << c_normal << ",";
    out << c_param << "param2" << c_normal << "=" << c_alias << "value2" << c_normal << ",...";
    out << "]//" << c_param << "uri" << c_normal << "\"" << std::endl;
    out << std::endl;
}

// assumes schemes is not empty
std::string HighestPriScheme(const std::map<FactoryInterface::Name,FactoryInterface::Precedence>& schemes)
{
    FactoryInterface::Name best_name;
    FactoryInterface::Precedence best_pri = std::numeric_limits<FactoryInterface::Precedence>::max();
    for(const auto& scheme : schemes) {
        if(scheme.second < best_pri) {
            best_name = scheme.first;
            best_pri = scheme.second;
        }
    }
    return best_name;
}

void PrintFactoryDetails(std::ostream& out, const std::string name, const pangolin::FactoryInterface& f, HelpVerbosity level, size_t indent, bool color)
{
    const std::string c_normal = color ? "\033[0m"  : "";
    const std::string c_scheme = color ? "\033[36m" : "";
    const std::string c_alias  = color ? "\033[32m" : "";
    const std::string c_params = color ? "\033[34m" : "";
    const std::string c_param  = color ? "\033[31m" : "";

    indent = std::max(indent, name.size());

    out << c_scheme;
    out << std::setw(indent);
    out << name << c_normal;
    out << "| " << f.Description() << std::endl;
    if(level >= HelpVerbosity::SYNOPSIS) {
        if(f.Schemes().size() > 1) {
            out << std::setw(indent) << " " << "| ";
            out << c_alias << "aliases: " << c_normal << "{";
            for(const auto& s : f.Schemes()) {
                out << c_scheme << s.first << c_normal << " (" << s.second << "); ";
            }
            out << "}" << std::endl;
        }
    }
    if(level >= HelpVerbosity::PARAMS) {
        if(f.Params().params.size()) {
            out << std::setw(indent) << " " << "| ";
            out << c_alias << "params: " << c_normal << std::endl;
            for(auto& param : f.Params().params) {
                out << std::setw(indent) << " " << "|   ";
                out << c_param << param.name_regex << c_normal << ":";
                if(!param.default_value.empty()) {
                    out << " (default='" << param.default_value << "')";
                } 
                out << std::endl;
                out << std::setw(indent) << " " << "|     ";
                out << param.description << std::endl;
            }
        }
    }
    if(level >= HelpVerbosity::SYNOPSIS) {
        std::cout << std::endl;
    }
}

void PrintFactoryRegistryDetails(
    std::ostream& out, const pangolin::FactoryRegistry& registry, std::type_index factory_type,
    const std::string& scheme_filter, HelpVerbosity level, bool color
) {
    const std::string c_normal = color ? "\033[0m"  : "";
    const std::string c_bold   = color ? "\033[1m"  : "";

    auto& factories = registry.GetFactories(factory_type);
    std::vector<std::string> high_pri_names;
    size_t longest_scheme = 0;
    for(const auto& f : factories) {
        const std::string scheme = HighestPriScheme(f->Schemes());
        high_pri_names.push_back(scheme);
        longest_scheme = std::max(longest_scheme, scheme.size());
    }
    
    // Create vector of indices which we will use to store alphabetical order
    std::vector<size_t> order(factories.size());
    std::iota(order.begin(), order.end(), 0);

    std::sort(order.begin(), order.end(), [&](size_t lhs, size_t rhs){
        return high_pri_names[lhs] < high_pri_names[rhs];
    });

    const std::string post = ":// ";
    longest_scheme += post.size();

    out << c_bold << "Factory Scheme List" << c_normal << std::endl << std::endl;

    if(level >= HelpVerbosity::SYNOPSIS) {
        out << "Factory schemes can be accessed through any of their named aliases (shown below). If multiple factories support the same scheme, the factory having the lowest scheme precedence (shown in brackets) will be used." << std::endl;
    }
    if(level >= HelpVerbosity::PARAMS) {
        out << "Available parameters and their default values for the factories are also shown. Regular expressions may be used if the parameter itself can vary (see the examples)."<< std::endl;
    }
    out << "The following is a list of factory provided schemes:" << std::endl;
    
    out << std::endl;

    for(size_t idx : order) {
        if(scheme_filter.empty() || factories[idx]->Schemes().count(scheme_filter) > 0) {
            PrintFactoryDetails(out, high_pri_names[idx] + post, *factories[idx], level, longest_scheme, color);
        }
    }

    if(level == 0) {
        out << std::endl;
    }
}

}
