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

///////////////////////////////////////////////////////////////////////////
// Subclass for textures

struct DeviceGlTexture : public DeviceTexture
{
    DeviceGlTexture(GLenum gl_target)
        : gl_target_(gl_target)
    {
    }

    ~DeviceGlTexture()
    {
        free();
    }

    ScopedBind<DeviceTexture> bind() const override
    {
        return {
            [t=this->gl_target_, id = this->gl_id_](){
                PANGO_GL(glBindTexture(t, id));
            },
            [t=this->gl_target_](){
                PANGO_GL(glBindTexture(t, 0));
            },
        };
    }

    sophus::ImageSize imageSize() const override
    {
        return image_size_;
    }

    void update(const Update& update) override
    {
        std::lock_guard<std::recursive_mutex> guard(buffer_mutex_);
        updates_.push_back(update);

        // for now, just sync immediately
        // TODO: implement the actual async upload
        sync();
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
                std::lock_guard<std::recursive_mutex> guard(buffer_mutex_);
                if(updates_.empty()) return;
                u = updates_.front();
                updates_.pop_front();
            }
            applyUpdateNow(u);
        }
    }

    void free()
    {
        if(gl_id_) PANGO_GL(glDeleteTextures(1, &gl_id_));
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
            image_size_ = {(int)size[0], (int)size[1]};

            free();
            PANGO_GL(glGenTextures(1, &gl_id_));
            PANGO_GL(glBindTexture(gl_target_, gl_id_));
            PANGO_GL(glTexImage2D(
                gl_target_, mip_level,
                gl_fmt.gl_sized_format,
                size[0], size[1], border,
                gl_fmt.gl_base_format,
                gl_fmt.gl_type, nullptr
            ));

            // TODO: set these from creation params
            if(true) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            }else{
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            }

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        // Upload data
        PANGO_GL(glBindTexture(gl_target_, gl_id_));
        PANGO_GL(glTexSubImage2D(
            GL_TEXTURE_2D, mip_level,
            u.dest_pos[0], u.dest_pos[1],
            src_size[0], src_size[1],
            gl_fmt.gl_base_format,
            gl_fmt.gl_type,
            u.src_data.get()
        ));
    }

    GLenum gl_target_ = 0;
    std::recursive_mutex buffer_mutex_;
    std::deque<DeviceBuffer::Update> updates_;
    RuntimePixelType data_type_;
    ImageSize image_size_ = {};
    GLuint gl_id_ = 0;

};

///////////////////////////////////////////////////////////////////////////
// Subclass for buffer objects

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
    std::mutex buffer_mutex_;
    std::deque<DeviceBuffer::Update> updates_;
    RuntimePixelType data_type_;
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
    FARM_FATAL("unreachable");
}

PANGO_CREATE(DeviceTexture) {
    const GLenum tex_target = GL_TEXTURE_2D;
    return Shared<DeviceGlTexture>::make(tex_target);
}

template<>
thread_local ScopedBind<DeviceTexture>* ScopedBind<DeviceTexture>::current = nullptr;

template<>
thread_local ScopedBind<DeviceBuffer>* ScopedBind<DeviceBuffer>::current = nullptr;


}
