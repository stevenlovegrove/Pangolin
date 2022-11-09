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

#include <pangolin/video/video.h>
#include <pangolin/video/video_output.h>
#include <pangolin/factory/factory_registry.h>
#include <pangolin/factory/RegisterFactoriesVideoInterface.h>
#include <pangolin/factory/RegisterFactoriesVideoOutputInterface.h>

namespace pangolin
{

std::unique_ptr<VideoInterface> OpenVideo(const std::string& str_uri)
{
    return OpenVideo( ParseUri(str_uri) );
}

std::unique_ptr<VideoInterface> OpenVideo(const Uri& uri)
{
    RegisterFactoriesVideoInterface();

    std::unique_ptr<VideoInterface> video =
            FactoryRegistry::I()->Construct<VideoInterface>(uri);

    if(!video) {
        throw VideoExceptionNoKnownHandler(uri.scheme);
    }

    return video;
}

std::unique_ptr<VideoOutputInterface> OpenVideoOutput(const std::string& str_uri)
{
    return OpenVideoOutput( ParseUri(str_uri) );
}

std::unique_ptr<VideoOutputInterface> OpenVideoOutput(const Uri& uri)
{
    RegisterFactoriesVideoOutputInterface();

    std::unique_ptr<VideoOutputInterface> video =
            FactoryRegistry::I()->Construct<VideoOutputInterface>(uri);

    if(!video) {
        throw VideoException("No known video handler for URI '" + uri.scheme + "', or device not found.");
    }

    return video;
}

}
