#pragma once

#include <pangolin/utils/shared.h>
#include <pangolin/gl/scoped_bind.h>
#include <sophus/image/runtime_image.h>

namespace pangolin
{

struct DeviceTexture
{
    virtual ~DeviceTexture() {};
    virtual ScopedBind<DeviceTexture> bind() const = 0;
    virtual sophus::ImageSize imageSize() const = 0;
    virtual sophus::RuntimePixelType pixelType() const = 0;

    // Update or initialize this texture or a subregion of it
    virtual void update(
        const sophus::IntensityImage<>& image,
        const Eigen::Array2i& destination = {0,0}
        ) = 0;

    // Returns true if this object is uninitialized and contains
    // no data or typed information
    virtual bool empty() = 0;

    struct Params {};
    static Shared<DeviceTexture> Create(Params p = {});
};


}
