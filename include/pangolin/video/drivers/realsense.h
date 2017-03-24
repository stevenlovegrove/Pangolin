#pragma once

#include <pangolin/pangolin.h>

#include <pangolin/video/video.h>

#include <pangolin/video/iostream_operators.h>

namespace rs {
class context;
class device;
}

namespace pangolin
{

//! Interface to video capture sources
struct RealSenseVideo : public VideoInterface, public VideoPropertiesInterface, public VideoPlaybackInterface
{
public:

    // Open all RGB and Depth streams from all devices
    RealSenseVideo(ImageDim dim=ImageDim(640,480), int fps=30);

    // Open streams specified
    // TODO
    //RealSenseVideo(std::vector<OpenNiStreamMode>& stream_modes);

    ~RealSenseVideo();

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

    //! Implement VideoPropertiesInterface::Properties()
    const picojson::value& DeviceProperties() const {
        return device_properties;
    }

    //! Implement VideoPropertiesInterface::Properties()
    const picojson::value& FrameProperties() const {
        return frame_properties;
    }

    //! Implement VideoPlaybackInterface::GetCurrentFrameId
    int GetCurrentFrameId() const;

    //! Implement VideoPlaybackInterface::GetTotalFrames
    int GetTotalFrames() const ;

    //! Implement VideoPlaybackInterface::Seek
    int Seek(int frameid);

protected:
    size_t sizeBytes;

    std::vector<StreamInfo> streams;

    picojson::value device_properties;
    picojson::value frame_properties;
    picojson::value* streams_properties;

    int current_frame_index;
    int total_frames;

    rs::context* ctx_;
    std::vector<rs::device*> devs_;

    ImageDim  dim_;
    int fps_;
};

}
