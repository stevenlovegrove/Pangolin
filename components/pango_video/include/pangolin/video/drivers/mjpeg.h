/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2013 Steven Lovegrove
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
#include <pangolin/image/typed_image.h>
#include <fstream>

namespace pangolin
{

// Video class that outputs test video signal.
class PANGOLIN_EXPORT MjpegVideo : public VideoInterface, public VideoPlaybackInterface
{
public:
    MjpegVideo(const std::string& filename);
    ~MjpegVideo();

    //! Implement VideoInput::Start()
    void Start() override;

    //! Implement VideoInput::Stop()
    void Stop() override;

    //! Implement VideoInput::SizeBytes()
    size_t SizeBytes() const override;

    //! Implement VideoInput::Streams()
    const std::vector<StreamInfo>& Streams() const override;

    //! Implement VideoInput::GrabNext()
    bool GrabNext( unsigned char* image, bool /*wait*/ = true ) override;

    //! Implement VideoInput::GrabNewest()
    bool GrabNewest( unsigned char* image, bool wait = true ) override;

    size_t GetCurrentFrameId() const override;
    size_t GetTotalFrames() const override;
    size_t Seek(size_t frameid) override;

protected:
    bool LoadNext();

    std::vector<StreamInfo> streams;
    size_t size_bytes;
    std::ifstream bFile;
    TypedImage next_image;
    std::vector<std::streampos> offsets;
    size_t next_frame_id;
};

}
