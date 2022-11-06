#include <pangolin/render/device_buffer.h>
#include <pangolin/context/factory.h>
#include <pangolin/gl/gl_type_info.h>

#include <deque>
#include <mutex>
#include <stdexcept>

using namespace sophus;

namespace pangolin
{

template<size_t kN, size_t kFrom>
std::array<size_t,kN> headAndZeros(
    const std::array<size_t,kFrom>& x
) {
    std::array<size_t,kN> ret;
    for(size_t i=0; i < kN; ++i) {
        ret[i] = x[i];
    }
    for(size_t i=kN; i < kFrom; ++i) {
        if(x[i] != 0) {
            throw std::runtime_error("Last Dim+1 .. N elements must be zero");
        }
    }
    return ret;
}

struct DeviceGlBuffer : public DeviceBuffer
{
    DeviceGlBuffer(GLenum buffer_type, GLenum gluse)
        : buffer_type_(buffer_type), gluse_(gluse)
    {
    }

    ~DeviceGlBuffer()
    {
        free();
    }

    void free()
    {
        if(gl_id_ != 0) glDeleteBuffers(1, &gl_id_);
    }

    ScopedBind<DeviceBuffer> bind() const override
    {
        return {
            [t=this->buffer_type_, id = this->gl_id_](){glBindBuffer(t, id);},
            [t=this->buffer_type_](){glBindBuffer(t, 0);},
        };
    }

    void update(const Data& data) override {
        std::lock_guard<std::mutex> guard(buffer_mutex_);
        updates_.push_back(data);
    }

    bool empty() override
    {
        return gl_id_ == 0;
    }

    void sync()
    {
        while(true) {
            Data u;
            {
                std::lock_guard<std::mutex> guard(buffer_mutex_);
                if(updates_.empty()) return;
                u = updates_.front();
                updates_.pop_front();
            }
            applyUpdateNow(u);
        }
    }

    void applyUpdateNow(Data& u)
    {
        const GlFormatInfo gl_fmt = glTypeInfo(u.data_type);

        if(gl_id_ == 0 /* || incompatible...  */) {
            data_type_ = u.data_type;
            num_elements_ = u.dest_element + u.num_elements;

            free();
            glGenBuffers(1, &gl_id_);
            glBindBuffer(buffer_type_, gl_id_);
            glBufferData(buffer_type_, sizeBytes(), nullptr, gluse_);
        }else{
            PANGO_CHECK(u.data_type == data_type_, "Attempting to upload {} format for {} texture.", u.data_type, data_type_);
            PANGO_CHECK(u.dest_element + u.num_elements <= num_elements_ );
        }

        glBindBuffer(buffer_type_, gl_id_);
        glBufferSubData(
            buffer_type_, elementSizeBytes() * u.dest_element,
            elementSizeBytes() * u.num_elements, u.data.get()
        );
    }

    size_t elementSizeBytes() const
    {
        return data_type_.num_channels * data_type_.num_bytes_per_pixel_channel;
    }

    size_t sizeBytes() const
    {
        return num_elements_ * elementSizeBytes();
    }

    GLenum buffer_type_;
    GLenum gluse_;
    std::mutex buffer_mutex_;
    std::deque<Data> updates_;
    RuntimePixelType data_type_;
    size_t num_elements_;
    GLuint gl_id_;

};

PANGO_CREATE(DeviceBuffer) {
    const GLenum bo_use = GL_DYNAMIC_DRAW;

    switch(p.kind) {
    case DeviceBuffer::Kind::VertexIndices:
        return Shared<DeviceGlBuffer>::make(
            GL_ELEMENT_ARRAY_BUFFER, bo_use
        );
    case DeviceBuffer::Kind::VertexAttributes:
        return Shared<DeviceGlBuffer>::make(
            GL_ARRAY_BUFFER, bo_use
        );
    };
    PANGO_UNREACHABLE();
}

template<>
thread_local ScopedBind<DeviceBuffer>* ScopedBind<DeviceBuffer>::current = nullptr;


}
