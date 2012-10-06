#ifndef PANGOLIN_OPENNI_H
#define PANGOLIN_OPENNI_H

#include <pangolin/pangolin.h>
#include <pangolin/video.h>

#include <XnCppWrapper.h>

namespace pangolin
{

enum OpenNiSensorType
{
    OpenNiUnassigned = -1,
    OpenNiRgb = 0,
    OpenNiIr = 1,
    OpenNiDepth = 2
};

//! Interface to video capture sources
struct OpenNiVideo : public VideoInterface
{
public:
    OpenNiVideo(OpenNiSensorType s1, OpenNiSensorType s2);
    ~OpenNiVideo();
    unsigned Width() const;
    unsigned Height() const;
    size_t SizeBytes() const;

    std::string PixFormat() const;

    void Start();
    void Stop();
    bool GrabNext( unsigned char* image, bool wait = true );
    bool GrabNewest( unsigned char* image, bool wait = true );

protected:
    xn::Context context;
    xn::DepthGenerator depthNode;
    xn::ImageGenerator imageNode;
    xn::IRGenerator irNode;

    OpenNiSensorType s1;
    OpenNiSensorType s2;
    size_t s1SizeBytes;
    size_t s2SizeBytes;
    size_t sizeBytes;
};

}

#endif // PANGOLIN_OPENNI_H
