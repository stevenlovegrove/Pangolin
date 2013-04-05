/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011-2013 Steven Lovegrove
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

#ifndef PANGOLIN_VIDEO_COMMON_H
#define PANGOLIN_VIDEO_COMMON_H

#include <pangolin/config.h>
#include <pangolin/type_convert.h>
#include <exception>
#include <string>
#include <map>

namespace pangolin
{

struct VideoException : std::exception
{
    VideoException(std::string str) : desc(str) {}
    VideoException(std::string str, std::string detail) {
        desc = str + "\n\t" + detail;
    }
    ~VideoException() throw() {}
    const char* what() const throw() { return desc.c_str(); }
    std::string desc;
};

struct VideoPixelFormat
{
    // Previously, VideoInterface::PixFormat returned a string.
    // For compatibility, make this string convertable
    inline operator std::string() const { return format; }
    
    std::string  format;
    unsigned int channels;
    unsigned int channel_bits[4];
    unsigned int bpp;
    bool planar;
};

struct Uri
{
    std::string scheme;
    std::string url;
    std::map<std::string,std::string> params;
    
    bool Contains(std::string key) {
        return params.find(key) != params.end();
    }
    
    template<typename T>
    T Get(std::string key, T default_val) {
        std::map<std::string,std::string>::iterator v = params.find(key);
        if(v != params.end()) {
            return Convert<T, std::string>::Do(v->second);
        }else{
            return default_val;
        }
    }
};

//! Parse string as Video URI
Uri ParseUri(std::string str_uri);

}

#endif // PANGOLIN_VIDEO_COMMON_H

