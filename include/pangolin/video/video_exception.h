#pragma once

#include <pangolin/platform.h>
#include <exception>
#include <string>

namespace pangolin {

struct PANGOLIN_EXPORT VideoException : std::exception
{
    VideoException(std::string str) : desc(str) {}
    VideoException(std::string str, std::string detail) {
        desc = str + "\n\t" + detail;
    }
    ~VideoException() throw() {}
    const char* what() const throw() { return desc.c_str(); }
    std::string desc;
};

}

