/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011 Steven Lovegrove
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <pangolin/platform.h>
#include <pangolin/utils/params.h>

#include <string>
#include <unordered_set>
#include <regex>

namespace pangolin
{

class ParamReader;

class PANGOLIN_EXPORT Uri : public Params
{
public:
    std::string scheme;
    std::string url;
    std::string full_uri;

    friend class ParamReader;
};

struct PANGOLIN_EXPORT Param {
    std::string name_regex;
    std::string default_value;
    std::string description;
};

struct PANGOLIN_EXPORT ParamSet {
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
        : param_set_(param_set),
            uri_(uri){}

    bool Contains( const std::string& param_name )
    {
        const Param* param = GetMatchingParamFromParamSet( param_name );
        if( param ){
            return uri_.Contains( param_name );
        }
        throw ParamReaderException( param_name );
    }

    template<typename T>
    T Get( const std::string& param_name ) const
    {
        const Param* param = GetMatchingParamFromParamSet( param_name );
        if( param ){
            return GetHelper(param_name, Convert<T,std::string>::Do(param->default_value));
        }
        throw ParamReaderException( param_name );
    }

    template<typename T>
    T Get( const std::string& param_name, const T& default_value ) const
    {
        const Param* param = GetMatchingParamFromParamSet( param_name );
        if( param ){
            return GetHelper(param_name, default_value);
        }
        throw ParamReaderException( param_name );
    }
    std::unordered_set<std::string> FindUnrecognizedUriParams();

private:
    template<typename T>
    T GetHelper( const std::string& param_name, const T& default_value ) const
    {
        return uri_.Get(param_name, default_value);
    }
    const Param* GetMatchingParamFromParamSet( const std::string& param_name ) const;
private:
    const ParamSet& param_set_;
    const Uri& uri_;
};

//! Parse string as Video URI
PANGOLIN_EXPORT
Uri ParseUri(const std::string& str_uri);

}
