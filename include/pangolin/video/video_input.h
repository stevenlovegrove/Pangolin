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

#include <pangolin/video/video_interface.h>
#include <pangolin/utils/uri.h>

namespace pangolin {

//! Generic wrapper class for different video sources
struct PANGOLIN_EXPORT VideoInput :
    public VideoInterface,
    public VideoFilterInterface
{
    VideoInput();
    VideoInput(const std::string& uri);
    ~VideoInput();

    void Open(const std::string& uri);
    void Reset();
    void Close();

    size_t SizeBytes() const;
    const std::vector<StreamInfo>& Streams() const;

    // Return details of first stream
    unsigned Width() const;
    unsigned Height() const;
    VideoPixelFormat PixFormat() const;
    const Uri& VideoUri() const;

    void Start();
    void Stop();
    bool GrabNext( unsigned char* image, bool wait = true );
    bool GrabNewest( unsigned char* image, bool wait = true );

    // Return pointer to inner video class as VideoType
    template<typename VideoType>
    VideoType* Cast() {
        return videos.size() ? dynamic_cast<VideoType*>(videos[0]) : 0;
    }

    // experimental - not stable
    bool Grab( unsigned char* buffer, std::vector<Image<unsigned char> >& images, bool wait = true, bool newest = false);

    std::vector<VideoInterface*>& InputStreams();

protected:
    Uri uri;
    std::unique_ptr<VideoInterface> src;
    std::vector<VideoInterface*> videos;
};

}
