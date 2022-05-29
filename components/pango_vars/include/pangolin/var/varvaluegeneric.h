/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2014 Steven Lovegrove
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

#include <string>
#include <memory>
#include <cmath>
#include <pangolin/utils/file_utils.h>

namespace pangolin
{
constexpr int META_FLAG_NONE      = 0x0000;
constexpr int META_FLAG_TOGGLE    = 0x0001;
constexpr int META_FLAG_READONLY  = 0x0002;

struct VarMeta
{
    VarMeta(
        const std::string& full_name = "",
        double min_val = 0.0, double max_val = 0.0,
        double increment = 0.0, int flags = META_FLAG_NONE,
        bool logscale = false, bool generic = false
    ) :
        full_name(full_name),
        increment(increment),
        flags(flags),
        gui_changed(false),
        logscale(logscale),
        generic(generic)
    {
        SetName(full_name);
        if(logscale) {
            if (min_val <= 0 || max_val <= 0) {
                throw std::runtime_error("LogScale: range of numbers must be positive!");
            }
            range[0] = std::log(min_val);
            range[1] = std::log(max_val);
        }else{
            range[0] = min_val;
            range[1] = max_val;
        }
    }

    void SetName(const std::string& full_name)
    {
        this->full_name = full_name;
        const std::vector<std::string> parts = pangolin::Split(full_name, '.');
        friendly = parts.size() > 0 ? parts[parts.size()-1] : "";
    }

    std::string full_name;
    std::string friendly;
    double range[2];
    double increment;
    int flags;
    bool gui_changed;
    bool logscale;
    bool generic;
};

// Forward declaration
template<typename T>
class VarValueT;

//! Abstract base class for named Pangolin variables
class VarValueGeneric
{
public:
    VarValueGeneric()
    {
    }

    virtual ~VarValueGeneric()
    {
    }

    virtual const char* TypeId() const = 0;
    virtual void Reset() = 0;
    virtual VarMeta& Meta() = 0;

//protected:
    // String serialisation object.
    std::shared_ptr<VarValueT<std::string>> str;
};

}
