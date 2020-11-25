#include <pangolin/utils/param_set.h>

namespace pangolin
{

bool ParamReader::Contains( const std::string& param_name )
{
    const ParamSet::Param* param = GetMatchingParamFromParamSet( param_name );
    if( param ){
        return uri_.Contains( param_name );
    }
    throw ParamReaderException( param_name );
}

std::unordered_set<std::string> ParamReader::FindUnrecognizedUriParams()
{
    std::unordered_set<std::string> result;
    for(const auto& param_pair: uri_.params){
        if(GetMatchingParamFromParamSet( param_pair.first ) == nullptr ){
            result.insert( param_pair.first );
        }
    }
    return result;
}

const ParamSet::Param* ParamReader::GetMatchingParamFromParamSet( const std::string& param_name ) const
{
    for(const auto& param : param_set_.params){
        std::regex name_regex( param.name_regex );
        if (std::regex_match ( param_name, name_regex )){
            return &param;
        }
    }
    return nullptr;
}

std::string ParamSet::str() const
{
    std::stringstream ss;
    if( params.size() > 0){
        ss << "[";
        size_t count = 0;
        for(const auto& param : params){
            ss << param.name_regex;
            if(!param.default_value.empty()){
                ss << "=" << param.default_value;
            }
            if(count < (params.size()-1)){
                ss << ",";
            }
            count++;
        }
        ss << "]";
    }
    return ss.str();
}

}
