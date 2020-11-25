#pragma once

#include <unordered_set>
#include <regex>

#include <pangolin/utils/uri.h>

namespace pangolin
{

struct PANGOLIN_EXPORT ParamSet {
    struct PANGOLIN_EXPORT Param {
        std::string name_regex;
        std::string default_value;
        std::string description;
    };

    std::vector<Param> params;
    std::string str() const;
};

class PANGOLIN_EXPORT ParamReader {
public:
    class ParamReaderException : public std::exception {
    public:
        ParamReaderException( const std::string &param_name )
        {
            error_message_ = param_name + " was not found in the parameter set";
        }
        virtual const char* what() const noexcept override {
            return error_message_.c_str();
        }
    private:
        std::string error_message_;
    };

    ParamReader( const ParamSet& param_set, const Uri& uri )
        : param_set_(param_set), uri_(uri)
    {}

    template<typename T>
    T Get( const std::string& param_name ) const
    {
        const ParamSet::Param* param = GetMatchingParamFromParamSet( param_name );
        if( param ){
            return GetHelper(param_name, Convert<T,std::string>::Do(param->default_value));
        }
        throw ParamReaderException( param_name );
    }

    template<typename T>
    T Get( const std::string& param_name, const T& default_value ) const
    {
        const ParamSet::Param* param = GetMatchingParamFromParamSet( param_name );
        if( param ){
            return GetHelper(param_name, default_value);
        }
        throw ParamReaderException( param_name );
    }

    bool Contains( const std::string& param_name );

    std::unordered_set<std::string> FindUnrecognizedUriParams();

private:
    template<typename T>
    T GetHelper( const std::string& param_name, const T& default_value ) const
    {
        return uri_.Get(param_name, default_value);
    }

    const ParamSet::Param* GetMatchingParamFromParamSet( const std::string& param_name ) const;

private:
    const ParamSet param_set_;
    const Uri uri_;
};

}
