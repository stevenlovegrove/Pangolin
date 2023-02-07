#pragma once

#include <pangolin/render/extra_pixel_traits.h>
#include <pangolin/utils/scoped_bind.h>
#include <pangolin/utils/shared.h>
#include <sophus/image/runtime_image_types.h>

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

struct DeviceBuffer {
  enum class Kind {
    VertexIndices,
    VertexAttributes,
    // ... add more
  };

  virtual ~DeviceBuffer(){};

  virtual ScopedBind<DeviceBuffer> bind() const = 0;

  // Returns true if this object is uninitialized and contains
  // no data or typed information
  virtual bool empty() const = 0;

  virtual sophus::RuntimePixelType dataType() const = 0;
  virtual size_t numElements() const = 0;

  struct UpdateParams {
    size_t dest_element = 0;
    size_t num_reserve_elements = 0;
  };
  struct Data {
    sophus::RuntimePixelType data_type;
    size_t num_elements = 0;
    std::shared_ptr<void> data;
    UpdateParams params = {};
  };

  /// Preconditions (panics if violated):
  ///
  ///   - u.data must not be null
  ///   - u.num_elements must be >= 1.
  virtual void pushToUpdateQueue(const Data& u) = 0;

  // Use with any contiguous, movable or copyable container with a
  // data() and size() method.
  template <ContainerWithDataMethod Container>
  void update(
      Container&& data,
      UpdateParams params = {.dest_element = 0, .num_reserve_elements = 0})
  {
    if (!data.data()) {
      return;
    }

    using T = std::decay_t<decltype(*data.data())>;
    const size_t num_elements = data.size();

    pushToUpdateQueue({
        .data_type = sophus::RuntimePixelType::fromTemplate<T>(),
        .num_elements = num_elements,
        .data = makeTypeErasedSharedPtr(data),
        .params = params,
    });
  }

  virtual void sync() const = 0;

  struct Params {
    Kind kind;
  };
  static Shared<DeviceBuffer> Create(Params p);

  template <typename T>
  static Shared<DeviceBuffer> Create(std::vector<T>& vec, Params p)
  {
    auto buffer = DeviceBuffer::Create(p);
    buffer->update(vec);
    return buffer;
  }

  template <typename T>
  static Shared<DeviceBuffer> Create(std::vector<T>&& vec, Params p)
  {
    auto buffer = DeviceBuffer::Create(p);
    buffer->update(std::move(vec));
    return buffer;
  }
};

}  // namespace pangolin
