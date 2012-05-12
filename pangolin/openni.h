#ifndef PANGOLIN_OPENNI_H
#define PANGOLIN_OPENNI_H

#include "pangolin.h"
#include "video.h"

#ifdef HAVE_OPENNI

#include <XnCppWrapper.h>

namespace pangolin
{

//! Interface to video capture sources
struct OpenNiVideo : public VideoInterface
{
public:
    OpenNiVideo();
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
};

}

#endif // HAVE_OPENNI
#endif // PANGOLIN_OPENNI_H
