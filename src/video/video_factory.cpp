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

#include <pangolin/video/video_factory.h>

namespace pangolin
{

VideoFactoryRegistry& VideoFactoryRegistry::I()
{
    static VideoFactoryRegistry instance;
    return instance;
}

VideoFactoryRegistry::~VideoFactoryRegistry()
{
    // nothing to do here.
}

void VideoFactoryRegistry::RegisterFactory(std::shared_ptr<VideoFactoryInterface> factory, uint32_t precedence, const std::string& scheme_name )
{
    FactoryItem item = {precedence, scheme_name, factory};
    factories.push_back( item );
    std::sort(factories.begin(), factories.end());
}

void VideoFactoryRegistry::UnregisterFactory(VideoFactoryInterface* factory)
{
    for( std::vector<FactoryItem>::iterator i = factories.end()-1; i != factories.begin(); --i)
    {
        if( i->factory.get() == factory ) {
            factories.erase(i);
        }
    }
}

void VideoFactoryRegistry::UnregisterAllFactories()
{
    factories.clear();
}

std::unique_ptr<VideoInterface> VideoFactoryRegistry::OpenVideo(const Uri& uri)
{
    // Iterate over all registered factories in order of precedence.
    for(auto& item : factories) {
        if( item.scheme == uri.scheme) {
            std::unique_ptr<VideoInterface> video = item.factory->OpenVideo(uri);
            if(video) {
                return video;
            }
        }
    }

    return std::unique_ptr<VideoInterface>();
}


}
