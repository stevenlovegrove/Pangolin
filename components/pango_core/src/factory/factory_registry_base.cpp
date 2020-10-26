#include "pangolin/factory/factory_registry.h"

namespace pangolin {
    std::vector<const FactoryRegistryBase*> FactoryRegistryBase::registry_list;

    FactoryRegistryBase::FactoryRegistryBase(const std::string &type_name)
        : type(type_name)
    {
        registry_list.push_back( this );
    }

    std::string FactoryHelpData::GetSynopsis()
    {
        std::stringstream ss;
        ss << scheme_ << ":" << param_set_.str() << "//";
        return ss.str();
    }
    std::string FactoryHelpData::GetDescription()
    {
        return description_;
    }
    std::vector<FactoryParamHelpData> FactoryHelpData::GetParamsHelp()
    {
        std::vector<FactoryParamHelpData> result;
        for(const auto& p:param_set_.params){
            FactoryParamHelpData data;
            data.name = p.name_regex;
            data.default_value = p.default_value;
            data.description = p.description;
            result.push_back( data );
        }
        return result;
    }
}
