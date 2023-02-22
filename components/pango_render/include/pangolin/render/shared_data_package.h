#pragma once

#include <pangolin/render/extra_pixel_traits.h>
#include <pangolin/utils/shared.h>
#include <sophus/image/dyn_image.h>

namespace pangolin
{

template <typename T>
concept ContainerWithDataMethod = requires(T)
{
  (void*)std::declval<T>().data();
};

template <ContainerWithDataMethod Container>
std::shared_ptr<void> makeTypeErasedSharedPtr(Container&& container)
{
  using C = std::decay_t<Container>;
  auto shared_data = std::make_shared<C>(std::forward<Container>(container));
  auto shared_void = std::shared_ptr<void>(shared_data, shared_data->data());
  return shared_void;
}

// Type erased shared ownership data
struct SharedDataPackage {
  SharedDataPackage() : num_elements(0) {}

  template <ContainerWithDataMethod Container>
  SharedDataPackage(Container&& container)
  {
    using T = std::decay_t<decltype(*container.data())>;
    num_elements = container.size();
    data_type = sophus::PixelFormat::fromTemplate<T>();
    data = makeTypeErasedSharedPtr(std::forward<Container>(container));
  }

  template <int kRows, int kCols>
  SharedDataPackage(Eigen::Matrix<float, kRows, kCols>&& matrix)
  {
    using C = Eigen::Matrix<float, kRows, kCols>;
    data_type = {
        .number_type = sophus::NumberType::floating_point,
        .num_channels = int(matrix.rows()),
        .num_bytes_per_pixel_channel = sizeof(float),
    };
    num_elements = matrix.cols();
    data = makeTypeErasedSharedPtr(std::forward<C>(matrix));
  }

  sophus::PixelFormat data_type;
  size_t num_elements;
  std::shared_ptr<void> data;
};

}  // namespace pangolin
