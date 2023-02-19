#pragma once

#include <pangolin/image/image.h>
#include <pangolin/image/pixel_format.h>
#include <sophus/image/dyn_image_types.h>

namespace pangolin
{

template <class TAllocator = Eigen::aligned_allocator<uint8_t>>
using IntensityImage = sophus::IntensityImage<TAllocator>;

}
