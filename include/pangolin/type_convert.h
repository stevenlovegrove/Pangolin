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

#ifndef PANGOLIN_TYPE_CONVERT_H
#define PANGOLIN_TYPE_CONVERT_H

#include <iostream>
#include <sstream>

namespace pangolin
{

struct BadInputException : std::exception {
    char const* what() const throw() { return "Failed to serialise type"; }
};

// Generic conversion through serialisation from / to string
template<typename T, typename S> struct Convert {
    static T Do(const S& src)
    {
        std::ostringstream oss;
        oss << src;
        std::istringstream iss(oss.str());
        T target;
        iss >> target;
        
        if(iss.fail())
            throw BadInputException();
        
        return target;
    }
};

// Apply bool alpha IO manipulator for bool types
template<> struct Convert<bool,std::string> {
    static bool Do(const std::string& src)
    {
        bool target;
        std::istringstream iss(src);
        iss >> target;
        
        if(iss.fail())
        {
            std::istringstream iss2(src);
            iss2 >> std::boolalpha >> target;
            if( iss2.fail())
                throw BadInputException();
        }
        
        return target;
    }
};

// From strings
template<typename T> struct Convert<T,std::string> {
    static T Do(const std::string& src)
    {
        T target;
        std::istringstream iss(src);
        iss >> target;
        
        if(iss.fail())
            throw BadInputException();
        
        return target;
    }
};

// To strings
template<typename S> struct Convert<std::string, S> {
    static std::string Do(const S& src)
    {
        std::ostringstream oss;
        oss << src;
        return oss.str();
    }
};

// Between strings is just a copy
template<> struct Convert<std::string, std::string> {
    static std::string Do(const std::string& src)
    {
        return src;
    }
};

}

#endif // PANGOLIN_TYPE_CONVERT_H
