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

#include <pangolin/utils/file_utils.h>
#include <pangolin/utils/uri.h>

#include <stdexcept>
#include <vector>
#include <string>

namespace pangolin
{

Uri ParseUri(const std::string &str_uri)
{
    Uri uri;
    uri.full_uri = str_uri;

    // Find Scheme delimiter
    size_t npos = 0;
    const size_t ns = str_uri.find(':', npos);
    if( ns != std::string::npos )
    {
        uri.scheme = str_uri.substr(0,ns);
        npos = ns+1;
    }else{
        uri.scheme = "file";
        uri.url = str_uri;
        return uri;
    }

    // Find Options delimiters
    if( str_uri.size() > npos && str_uri[npos] == '[' )
    {
        const size_t nob = npos;
        const size_t ncb = str_uri.find(']', nob+1);
        if(ncb != std::string::npos)
        {
            const std::string queries = str_uri.substr(nob+1, ncb - (ns+2) );
            std::vector<std::string> params;
            Split(queries, ',', params);
            for(size_t i=0; i< params.size(); ++i)
            {
                std::vector<std::string> args;
                Split(params[i], '=', args );
                std::string key = Trim(args[0]);
                std::string val = args.size() > 1 ? Trim(args[1]) : "";
                uri.Set(key,val);
            }
        }else{
            throw std::runtime_error("Unable to parse URI: '" + str_uri + "'");
        }
        npos = ncb + 1;
    }

    // Find url delimiter
    size_t nurl = str_uri.find("//", npos);
    if(nurl != std::string::npos)
    {
        uri.url = str_uri.substr(nurl+2);
    }

    return uri;
}

std::ostream& operator<< (std::ostream &out, Uri &uri)
{
    out << "scheme: " << uri.scheme << std::endl;
    out << "url:    " << uri.url << std::endl;
    out << "params:" << std::endl;

    for( Uri::ParamMap::const_iterator ip = uri.params.begin();
         ip != uri.params.end(); ++ip)
    {
        out << "\t" << ip->first << " = " << ip->second << std::endl;
    }

    return out;
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

const Param* ParamReader::GetMatchingParamFromParamSet( const std::string& param_name ) const
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
} // pangolin
