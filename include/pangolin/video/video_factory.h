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

#pragma once

#include <memory>
#include <pangolin/video/video.h>
#include <pangolin/utils/static_init.h>

namespace pangolin
{

class VideoFactoryInterface
{
public:
    virtual std::unique_ptr<VideoInterface> OpenVideo(const Uri& video_uri) = 0;
};

class VideoFactoryRegistry
{
public:
    static VideoFactoryRegistry& I();

    ~VideoFactoryRegistry();

    void RegisterFactory(std::shared_ptr<VideoFactoryInterface> factory, uint32_t precedence, const std::string& scheme_name );

    void UnregisterFactory(VideoFactoryInterface* factory);

    void UnregisterAllFactories();

    std::unique_ptr<VideoInterface> OpenVideo(const Uri& uri);

private:
    struct FactoryItem
    {
        uint32_t precedence;
        std::string scheme;
        std::shared_ptr<VideoFactoryInterface> factory;

        bool operator<(const FactoryItem& rhs) const {
            return precedence < rhs.precedence;
        }
    };

    // Priority, Factory tuple
    std::vector<FactoryItem> factories;
};

#define PANGOLIN_REGISTER_FACTORY(x) PANGOLIN_STATIC_CONSTRUCTOR( Register ## x ## Factory )

}
