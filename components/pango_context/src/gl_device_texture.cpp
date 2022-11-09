#include <pangolin/context/factory.h>
#include <pangolin/render/device_texture.h>
#include <pangolin/gl/gl_type_info.h>

#include <deque>
#include <mutex>
#include <stdexcept>

using namespace sophus;

namespace pangolin
{

struct DeviceGlTexture : public DeviceTexture
{
    struct Update
    {
        IntensityImage<> image;
        Eigen::Array2i destination;
    };

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

    RuntimePixelType pixelType() const override
    {
        return data_type_;
    }

    void update(
        const sophus::IntensityImage<>& image,
        const Eigen::Array2i& destination = {0,0}
    ) override {
        std::lock_guard<std::recursive_mutex> guard(buffer_mutex_);
        updates_.push_back(Update{image, destination});

        // TODO: we'll service this queue in another thread eventually...
        sync();
    }

    bool empty() const override
    {
        return gl_id_ == 0;
    }

    void sync() const override
    {
        while(true) {
            Update u;
            {
                std::lock_guard<std::recursive_mutex> guard(buffer_mutex_);
                if(updates_.empty()) return;
                u = updates_.front();
                updates_.pop_front();
            }
            applyUpdateNow(u);
        }
    }

    void free() const
    {
        if(gl_id_) PANGO_GL(glDeleteTextures(1, &gl_id_));
    }

    void applyUpdateNow(Update& u) const
    {
        constexpr GLint mip_level = 0;
        constexpr GLint border = 0;

        const RuntimePixelType data_type = u.image.pixelType();
        const GlFormatInfo gl_fmt = glTypeInfo(data_type);

        // Create memory for resource if needed
        if(gl_id_ == 0 /* || incompatible...  */) {
            PANGO_ASSERT(!u.image.isEmpty());

            data_type_ = data_type;
            image_size_ = u.image.imageSize();

            free();
            PANGO_GL(glGenTextures(1, &gl_id_));
            PANGO_GL(glBindTexture(gl_target_, gl_id_));
            PANGO_GL(glTexImage2D(
                gl_target_, mip_level,
                gl_fmt.gl_sized_format,
                image_size_.width, image_size_.height,
                border, gl_fmt.gl_base_format,
                gl_fmt.gl_type, nullptr
            ));

            // TODO: set these from creation params
            if(true) {
                PANGO_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
                PANGO_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            }else{
                PANGO_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
                PANGO_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
            }

            PANGO_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            PANGO_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        }else{
            PANGO_CHECK(data_type == data_type_, "Attempting to upload {} format for {} texture.", data_type, data_type_);
        }

        // Upload data
        const size_t stride = u.image.pitchBytes() / data_type.bytesPerPixel();
        PANGO_CHECK(stride * data_type.bytesPerPixel() == u.image.pitchBytes(),
            "Image pitch is not a multiple of the pixel size (which OpenGL requires).");
        PANGO_GL(glPixelStorei(GL_UNPACK_ROW_LENGTH, stride));
        PANGO_GL(glPixelStorei( GL_UNPACK_ALIGNMENT, 1));
        PANGO_GL(glBindTexture(gl_target_, gl_id_));
        PANGO_GL(glTexSubImage2D(
            GL_TEXTURE_2D, mip_level,
            u.destination.x(), u.destination.y(),
            u.image.width(), u.image.height(),
            gl_fmt.gl_base_format,
            gl_fmt.gl_type,
            u.image.rawPtr()
        ));
        // return to default.
        PANGO_GL(glPixelStorei(GL_UNPACK_ROW_LENGTH, 0));
        // (we'll leave GL_UNPACK_ALIGNMENT since thats the only sane default).
    }

    GLenum gl_target_ = 0;
    mutable std::recursive_mutex buffer_mutex_;
    mutable std::deque<Update> updates_;
    mutable RuntimePixelType data_type_;
    mutable ImageSize image_size_ = {};
    mutable GLuint gl_id_ = 0;

};

PANGO_CREATE(DeviceTexture) {
    const GLenum tex_target = GL_TEXTURE_2D;
    return Shared<DeviceGlTexture>::make(tex_target);
}

template<>
ScopedBind<DeviceTexture>::pScopedBind& ScopedBind<DeviceTexture>::getLocalActiveScopePtr()
{
    static thread_local pScopedBind x = nullptr;
    return x;
}

}
