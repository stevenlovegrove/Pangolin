#pragma once

#include <pangolin/image/image.h>
#include <pangolin/image/pixel_format.h>
#include <sophus/image/runtime_image.h>

namespace pangolin
{

template <template <typename> class TAllocator = Eigen::aligned_allocator>
using IntensityImage = sophus::IntensityImage<TAllocator>;

}
