#include <pangolin/render/device_buffer.h>
#include <pangolin/context/factory.h>
#include <pangolin/gl/glplatform.h>

#include <deque>
#include <mutex>
#include <stdexcept>

using namespace sophus;

namespace pangolin
{

template<typename T>
bool between(const T& val, const T& low_inclusive, const T& high_inclusive)
{
    return low_inclusive <= val && val <= high_inclusive;
}

template<typename T>
bool isOneOf(const T& val, const std::initializer_list<T>& of)
{
    for(auto x : of) {
        if(val == x) return true;
    }
    return false;
}

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

// Struct representing a sized form and the corresponding
// base format and component type
// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml
struct GlFormatInfo
{
    // e.g. GL_R8, GL_R16, ..., GL_RGBA32F
    GLint gl_sized_format;

    // e.g. GL_RED, GL_RG, GL_RGB, GL_RGBA
    GLint gl_base_format;

    // GL_BYTE, GL_UNSIGNED_SHORT, ...
    GLint gl_type;
};

GlFormatInfo glTypeInfo(const RuntimePixelType& pixel_type)
{
    constexpr static GLint type_table[] = {
        0,       // unspecified
        GL_BYTE,
        GL_UNSIGNED_SHORT,
        GL_UNSIGNED_INT,
        GL_FLOAT
    };

    // inner stride is GL_RED, GL_RG, GL_RGB, GL_RGBA
    // outer stride is format type
    constexpr static GLint format_table[][4] = {
        {GL_RED, GL_RG, GL_RGB, GL_RGBA},               // base (untyped)
        {GL_R8UI, GL_RG8UI, GL_RGB8UI, GL_RGBA8UI},     // uint8_t
        {GL_R16UI, GL_RG16UI, GL_RGB16UI, GL_RGBA16UI}, // uint16_t
        {GL_R32UI, GL_RG32UI, GL_RGB32UI, GL_RGBA32UI}, // uint32_t
        {GL_R32F, GL_RG32F, GL_RGB32F, GL_RGBA32F},     // float (32bits)
    };

    // Make sure we'll be in bounds...
    const int cidx = pixel_type.num_channels;
    const int nbytes =pixel_type.num_bytes_per_pixel_channel;
    const bool bfixed = pixel_type.number_type == sophus::NumberType::fixed_point;

    if( between(cidx, 1, 4) && isOneOf(nbytes, {1,2,4}) ) {
        const int fidx = nbytes < 4 ? nbytes+1 : (bfixed?4:5);
        return {
            .gl_sized_format = format_table[fidx][cidx],
            .gl_base_format = format_table[0][cidx],
            .gl_type = type_table[fidx]
        };
    }

    throw std::runtime_error("Unsupported image type");
}

struct DeviceBufferGl : public DeviceBuffer
{
    DeviceBufferGl()
        : gl_id_(0)
    {
    }

    void update(const Update& update) override
    {
        std::lock_guard<std::mutex> guard(buffer_mutex_);
        updates_.push_back(update);
    }

    bool empty() override
    {
        return gl_id_ == 0;
    }

    void sync() override
    {
        while(true) {
            DeviceBuffer::Update u;
            {
                std::lock_guard<std::mutex> guard(buffer_mutex_);
                if(updates_.empty()) return;
                u = updates_.front();
                updates_.pop_front();
            }
            applyUpdateNow(u);
        }
    }

    virtual void applyUpdateNow(DeviceBuffer::Update& u) = 0;

protected:
    std::mutex buffer_mutex_;
    std::deque<DeviceBuffer::Update> updates_;
    RuntimePixelType data_type_;
    GLuint gl_id_;
};

struct DeviceBufferGlTexture : public DeviceBufferGl
{
    DeviceBufferGlTexture(GLenum gl_target)
        : gl_target_(gl_target)
    {
    }

    ~DeviceBufferGlTexture()
    {
        free();
    }

    void free()
    {
        if(gl_id_) glDeleteTextures(1, &gl_id_);
    }

    void applyUpdateNow(DeviceBuffer::Update& u)
    {
        constexpr GLint mip_level = 0;
        constexpr GLint border = 0;

        const GlFormatInfo gl_fmt = glTypeInfo(u.data_type);
        const auto src_size = headAndZeros<2>(u.src_sizes);

        // Create memory for resource if needed
        if(gl_id_ == 0 /* || incompatible...  */) {
            auto size = headAndZeros<2>(u.dest_sizes);
            if(!size[0] && !size[1]) size = src_size;
            if(!size[0] || !size[1])
                throw std::invalid_argument("Image area cannot be 0");

            data_type_ = u.data_type;

            free();
            glGenTextures(1, &gl_id_);
            glBindTexture(gl_target_, gl_id_);
            glTexImage2D(
                gl_target_, mip_level,
                gl_fmt.gl_sized_format,
                size[0], size[1], border,
                gl_fmt.gl_base_format,
                gl_fmt.gl_type, nullptr
            );
        }

        // Upload data
        glBindTexture(gl_target_, gl_id_);
        glTexSubImage2D(
            GL_TEXTURE_2D, mip_level,
            u.dest_pos[0], u.dest_pos[1],
            src_size[0], src_size[1],
            gl_fmt.gl_base_format,
            gl_fmt.gl_type,
            u.src_data.get()
        );
    }

    GLenum gl_target_;
};

struct DeviceBufferGlBufferObject : public DeviceBufferGl
{
    DeviceBufferGlBufferObject(GLenum buffer_type, GLenum gluse)
        : buffer_type_(buffer_type), gluse_(gluse)
    {
    }

    ~DeviceBufferGlBufferObject()
    {
        free();
    }

    void free()
    {
        if(gl_id_ != 0) glDeleteBuffers(1, &gl_id_);
    }

    void applyUpdateNow(DeviceBuffer::Update& u)
    {
        const size_t src_size = headAndZeros<1>(u.src_sizes)[0];

        if(gl_id_ == 0 /* || incompatible...  */) {
            size_t size = headAndZeros<1>(u.dest_sizes)[0];
            if(size==0) size = src_size;
            if(size==0) throw std::invalid_argument("Buffer size cannot be 0");
            data_type_ = u.data_type;
            const size_t size_bytes = size * data_type_.num_channels * data_type_.num_bytes_per_pixel_channel;

            free();
            glGenBuffers(1, &gl_id_);
            glBindBuffer(buffer_type_, gl_id_);
            glBufferData(buffer_type_, size_bytes, nullptr, gluse_);
        }
    }

    GLenum buffer_type_;
    GLenum gluse_;
};

PANGO_CREATE(DeviceBuffer) {
    const GLenum bo_use = GL_DYNAMIC_DRAW;
    const GLenum tex_target = GL_TEXTURE_2D;

    switch(p.kind) {
        case DeviceBuffer::Kind::Texture:
            return Shared<DeviceBufferGlTexture>::make(tex_target);
        case DeviceBuffer::Kind::VertexIndices:
            return Shared<DeviceBufferGlBufferObject>::make(
                GL_ELEMENT_ARRAY_BUFFER, bo_use
            );
        case DeviceBuffer::Kind::VertexAttributes:
            return Shared<DeviceBufferGlBufferObject>::make(
                GL_ARRAY_BUFFER, bo_use
            );
    };
}

}
