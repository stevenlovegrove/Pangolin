#pragma once

#include <pangolin/utils/shared.h>
#include <pangolin/gl/scoped_bind.h>
#include <sophus/image/runtime_image.h>
#include <pangolin/render/extra_pixel_traits.h>

namespace pangolin
{

struct DeviceBuffer
{
    enum class Kind
    {
        VertexIndices,
        VertexAttributes,
        // ... add more
    };

    virtual ~DeviceBuffer() {};

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
        std::shared_ptr<void> data;
        sophus::RuntimePixelType data_type;
        size_t num_elements = 0;
        UpdateParams params = {};
    };
    virtual void update(const Data& data) = 0;

    // Use with any contiguous, movable or copyable container with a
    // data() and size() method.
    template<typename Container>
    void update(Container&& data, UpdateParams params) {
        using C = std::remove_cvref_t<Container>;
        using T = std::remove_cvref_t<std::remove_pointer_t<decltype(data.data())>>;
        auto shared_data = std::make_shared<C>(std::forward<Container>(data));

        update({
            .data = std::shared_ptr<void>(shared_data, shared_data->data()),
            .data_type = sophus::RuntimePixelType::fromTemplate<T>(),
            .num_elements = shared_data->size(),
            .params = params,
        });
    }

    virtual void sync() const = 0;

    struct Params { Kind kind; };
    static Shared<DeviceBuffer> Create(Params p);

    template<typename T>
    static Shared<DeviceBuffer> Create(std::vector<T>& vec, Params p) {
        auto buffer = DeviceBuffer::Create(p);
        buffer->update(vec);
        return buffer;
    }

    template<typename T>
    static Shared<DeviceBuffer> Create(std::vector<T>&& vec, Params p) {
        auto buffer = DeviceBuffer::Create(p);
        buffer->update(std::move(vec));
        return buffer;
    }
};

}
