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

#include <pangolin/video/video.h>
#include <pangolin/log/packetstream_reader.h>

namespace pangolin
{

class PANGOLIN_EXPORT PangoVideo
    : public VideoInterface, public VideoPropertiesInterface, public VideoPlaybackInterface
{
public:
    PangoVideo(const std::string& filename, bool realtime = true);
    ~PangoVideo();

    // Implement VideoInterface

    size_t SizeBytes() const override;

    const std::vector<StreamInfo>& Streams() const override;

    void Start() override;

    void Stop() override;

    bool GrabNext( unsigned char* image, bool wait = true ) override;

    bool GrabNewest( unsigned char* image, bool wait = true ) override;

    // Implement VideoPropertiesInterface
    const picojson::value& DeviceProperties() const override {
        if (-1 == _src_id) throw std::runtime_error("Not initialised");
        return _device_properties;
    }

    const picojson::value& FrameProperties() const override {
        return _frame_properties;
    }

    // Implement VideoPlaybackInterface

    int GetCurrentFrameId() const override;

    int GetTotalFrames() const override;

    int Seek(int frameid) override;

private:
    void HandlePipeClosed();

protected:
    int FindSource();

    PacketStreamReader _reader;
    SyncTime _realtime_sync;

    size_t _size_bytes;
    std::vector<StreamInfo> _streams;
    picojson::value _device_properties;
    picojson::value _frame_properties;
    int _src_id;
    const std::string _filename;
    bool _realtime;
    bool _is_pipe;
    bool _is_pipe_open;
    int _pipe_fd;
};

}
