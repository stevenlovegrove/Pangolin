#pragma once

#include <pangolin/utils/shared.h>
#include <pangolin/gl/scoped_bind.h>
#include <sophus/image/runtime_image.h>

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

    struct Data {
        std::shared_ptr<void> data;
        sophus::RuntimePixelType data_type;
        size_t num_elements = 0;
        size_t dest_element = 0;
    };
    virtual void update(const Data& data) = 0;

    template<typename T>
    void update(const std::vector<T>& data, size_t dest_element = 0) {
        // Copy
        auto shared_data = std::make_shared<std::vector<T>>(data);

        update({
            .data = std::shared_ptr<void>(shared_data, shared_data->data()),
            .data_type = sophus::RuntimePixelType::fromTemplate<T>(),
            .num_elements = shared_data->size(),
            .dest_element = dest_element,
        });
    }

    template<typename T>
    void update(std::vector<T>&& data, size_t dest_element = 0) {
        // Move
        auto shared_data = std::make_shared<std::vector<T>>(std::move(data));

        update({
            .data = std::shared_ptr<void>(shared_data, shared_data->data()),
            .data_type = sophus::RuntimePixelType::fromTemplate<T>(),
            .num_elements = shared_data->size(),
            .dest_element = dest_element,
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
