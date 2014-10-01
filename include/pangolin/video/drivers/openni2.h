/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2014 Richard Newcombe
 *               2014 Steven Lovegrove
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

#ifndef PANGOLIN_OPENNI2_H
#define PANGOLIN_OPENNI2_H

#include <pangolin/pangolin.h>

#include <pangolin/video/video.h>
#include <pangolin/video/drivers/openni_common.h>

#include <OpenNI.h>

namespace pangolin
{

//! Interface to video capture sources
struct OpenNiVideo2 : public VideoInterface, public VideoPropertiesInterface
{
public:
    OpenNiVideo2(
        OpenNiSensorType s1, OpenNiSensorType s2,
        ImageDim dim, int fps, bool realtime = true
    );

    void UpdateProperties();

    void SetDepthCloseRange(bool enable);
    void SetDepthHoleFilter(bool enable);
    void SetDepthColorSyncEnabled(bool enable);
    void SetRegisterDepthToImage(bool enable);

    ~OpenNiVideo2();

    //! Implement VideoInput::Start()
    void Start();

    //! Implement VideoInput::Stop()
    void Stop();

    //! Implement VideoInput::SizeBytes()
    size_t SizeBytes() const;

    //! Implement VideoInput::Streams()
    const std::vector<StreamInfo>& Streams() const;

    //! Implement VideoInput::GrabNext()
    bool GrabNext( unsigned char* image, bool wait = true );

    //! Implement VideoInput::GrabNewest()
    bool GrabNewest( unsigned char* image, bool wait = true );

    //! Implement VideoInput::Properties()
    const json::value& DeviceProperties() const {
        return device_properties;
    }

    //! Implement VideoInput::Properties()
    const json::value& FrameProperties() const {
        return frame_properties;
    }

protected:
    void PrintOpenNI2Modes(openni::SensorType sensorType);

    openni::VideoMode FindOpenNI2Mode(openni::SensorType sensorType,
        int width, int height,
        int fps, openni::PixelFormat fmt
    );

    std::vector<StreamInfo> streams;
    json::value device_properties;
    json::value frame_properties;
    json::value* streams_properties;

    OpenNiSensorType sensor_type[2];

    size_t sizeBytes;

    bool use_depth;
    bool use_ir;
    bool use_rgb;
    bool depth_to_color;
    bool use_ir_and_rgb;
    bool fromFile;
    openni::Device device;
    openni::VideoStream video_stream[2];
    openni::VideoFrameRef video_frame[2];
};

}

#endif // PANGOLIN_OPENNI2_H
