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
#include <pangolin/utils/type_convert.h>

#include <string>
#include <vector>
#include <algorithm>

namespace pangolin
{

struct PANGOLIN_EXPORT Params
{
    // Why not an actual map here?
    // Well, in some drivers (notably USB3Vision based drivers) order
    // matters, sometimes with duplicate keys.
    typedef std::pair<std::string,std::string> KeyValue;
    typedef std::vector<KeyValue> ParamMap;

    Params()
    {
    }

    Params(std::initializer_list<std::pair<std::string,std::string>> l)
        : params(l)
    {
    }

    // \returns true iff any entry has the key \param key
    bool Contains(const std::string& key) const
    {
        for(ParamMap::const_iterator it = params.begin(); it!=params.end(); ++it) {
            if(it->first == key) return true;
        }
        return false;
    }

    // \returns the value of the last entry with key \param key
    //          or \param default_val if no such entry exists
    template<typename T>
    T Get(const std::string& key, T default_val) const
    {
        // Return last value passed to the key.
        for(ParamMap::const_reverse_iterator it = params.rbegin(); it!=params.rend(); ++it) {
            if(it->first == key) return Convert<T, std::string>::Do(it->second);
        }
        return default_val;
    }

    // Adds a new entry with key \param key and value \param val to the end of the parameters list
    // note: Any existing entry with key \param key will remain, but be overriden users with map-semantics.
    template<typename T>
    void Set(const std::string& key, const T& val)
    {
        params.push_back(std::pair<std::string,std::string>(key,Convert<std::string,T>::Do(val)));
    }

    // Remove all entries with key \param key
    void Remove(const std::string& key)
    {
        params.erase(std::remove_if(params.begin(), params.end(), [&key](const KeyValue& kv){
            return kv.first == key;
        }), params.end());
    }

    ParamMap params;
};

}
