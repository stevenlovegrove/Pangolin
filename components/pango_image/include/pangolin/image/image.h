#pragma once

#include <sophus/image/image.h>

namespace pangolin
{

    template<typename T>
    using ImageView = sophus::ImageView<T>;

    template<typename T>
    using MutImageView = sophus::MutImageView<T>;

    template<typename T>
    using Image = sophus::Image<T>;

    template<typename T>
    using MutImage = sophus::MutImage<T>;

    using ImageSize = sophus::ImageSize;

    using ImageShape = sophus::ImageShape;

}
