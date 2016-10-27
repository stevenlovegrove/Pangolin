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
#include <pangolin/video/video_factory.h>

namespace pangolin
{

std::unique_ptr<VideoInterface> OpenVideo(const std::string& str_uri)
{
    return OpenVideo( ParseUri(str_uri) );
}

std::unique_ptr<VideoInterface> OpenVideo(const Uri& uri)
{
    std::unique_ptr<VideoInterface> video = VideoFactoryRegistry::I().OpenVideo(uri);

    if(!video) {
        throw VideoException("No known video handler for URI '" + uri.scheme + "'");
    }

    return video;
}

VideoInput::VideoInput()
{
}

VideoInput::VideoInput(const std::string& uri)
{
    Open(uri);
}

VideoInput::~VideoInput()
{
    Close();
}

void VideoInput::Open(const std::string& sUri)
{
    uri = ParseUri(sUri);
    
    Close();
    
    // Create video device
    src = OpenVideo(uri);
    videos.push_back(src.get());
}

void VideoInput::Reset()

{
    Close();

    // Create video device
    src = OpenVideo(uri);
    videos.push_back(src.get());
}

void VideoInput::Close()
{
    videos.clear();
}

size_t VideoInput::SizeBytes() const
{
    if( !videos.size() ) throw VideoException("No video source open");
    return videos[0]->SizeBytes();
}

const std::vector<StreamInfo>& VideoInput::Streams() const
{
    if( !videos.size() ) throw VideoException("No video source open");
    return videos[0]->Streams();
}

unsigned int VideoInput::Width() const
{
    if( !videos.size() ) throw VideoException("No video source open");
    return (unsigned int)videos[0]->Streams()[0].Width();
}

unsigned int VideoInput::Height() const
{
    if( !videos.size() ) throw VideoException("No video source open");
    return (unsigned int)videos[0]->Streams()[0].Height();
}

VideoPixelFormat VideoInput::PixFormat() const
{
    if( !videos.size() ) throw VideoException("No video source open");
    return Streams()[0].PixFormat();
}

const Uri& VideoInput::VideoUri() const
{
    return uri;
}

void VideoInput::Start()
{
    if( !videos.size() ) throw VideoException("No video source open");
    videos[0]->Start();
}

void VideoInput::Stop()
{
    if( !videos.size() ) throw VideoException("No video source open");
    videos[0]->Stop();
}

bool VideoInput::GrabNext( unsigned char* image, bool wait )
{
    if( !videos.size() ) throw VideoException("No video source open");
    return videos[0]->GrabNext(image,wait);
}

bool VideoInput::GrabNewest( unsigned char* image, bool wait )
{
    if( !videos.size() ) throw VideoException("No video source open");
    return videos[0]->GrabNewest(image,wait);
}

bool VideoInput::Grab( unsigned char* buffer, std::vector<Image<unsigned char> >& images, bool wait, bool newest)
{
    bool success;
    
    if(newest) {
        success = GrabNewest(buffer, wait);
    }else{
        success = GrabNext(buffer, wait);
    }
    
    if(success) {
        images.clear();
        for(size_t s=0; s < Streams().size(); ++s) {
            images.push_back(Streams()[s].StreamImage(buffer));
        }
    }
    
    return success;
}

std::vector<VideoInterface*>& VideoInput::InputStreams()
{
    return videos;
}

}
