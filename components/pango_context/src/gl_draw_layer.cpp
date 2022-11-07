#include <pangolin/gui/draw_layer.h>
#include <pangolin/context/context.h>
#include <pangolin/context/factory.h>
#include <pangolin/handler/handler.h>
#include <pangolin/render/gl_vao.h>
#include <pangolin/gl/glsl_program.h>
#include <pangolin/gl/uniform.h>
#include <pangolin/gl/gl.h>
#include <pangolin/utils/variant_overload.h>
#include <pangolin/maths/camera_look_at.h>
#include <pangolin/maths/projection.h>
#include <pangolin/var/var.h>

#include "gl_utils.h"

#include <unordered_map>

namespace pangolin
{

constexpr GLenum toGlEnum (DrawnPrimitives::Type type) {
    switch(type) {
        case DrawnPrimitives::Type::points: return GL_POINTS;
        case DrawnPrimitives::Type::line_strip: return GL_LINE_STRIP;
        case DrawnPrimitives::Type::line_loop: return GL_LINE_LOOP;
        case DrawnPrimitives::Type::triangles: return GL_TRIANGLES;
        case DrawnPrimitives::Type::triangle_strip: return GL_TRIANGLE_STRIP;
        default: return 0;
    }
}

struct DrawnPrimitivesProgram
{
    void draw(
        const DrawnPrimitives& primitives,
        const sophus::CameraModel& camera,
        const sophus::Se3F64& cam_from_world,
        MinMax<double> near_far
    ) {
        if(!primitives.vertices->empty()) {
            auto bind_prog = prog->bind();
            auto bind_vao = vao.bind();
            if(camera.distortionType() != sophus::CameraDistortionType::pinhole) {
                PANGO_WARN("Ignoring distortion component of camera for OpenGL rendering for now.");
            }
            u_intrinsics = projectionClipFromCamera(
                camera.imageSize(), camera.focalLength(),
                camera.principalPoint(), near_far
            ).cast<float>();
            u_cam_from_world = (cam_from_world.matrix() * primitives.world_from_drawable).cast<float>();
            u_color = primitives.default_color.cast<float>();
            vao.addVertexAttrib(0, *primitives.vertices);
            PANGO_GL(glPointSize(primitives.default_radius*2.0f));
            PANGO_GL(glDrawArrays(toGlEnum(primitives.element_type), 0, primitives.vertices->numElements()));
        }
    }

private:
    const Shared<GlSlProgram> prog = GlSlProgram::Create({
        .sources = {{ .origin="/components/pango_opengl/shaders/main_primitives_points.glsl" }}
    });
    GlVertexArrayObject vao = {};
    const GlUniform<Eigen::Matrix4f> u_intrinsics = {"proj"};
    const GlUniform<Eigen::Matrix4f> u_cam_from_world = {"cam_from_world"};
    const GlUniform<Eigen::Vector4f> u_color = {"color"};
};

struct DrawnImageProgram
{
    void draw(
        const DrawnImage& drawn_image,
        const sophus::CameraModel& camera,
        const sophus::Se3F64& cam_from_world,
        MinMax<double> near_far
    ) {
        // ensure we're synced
        drawn_image.image->sync();

        PANGO_GL(glActiveTexture(GL_TEXTURE0));
        auto bind_im = drawn_image.image->bind();
        auto bind_prog = prog->bind();
        auto bind_vao = vao.bind();

        u_image_size = Eigen::Vector2f(camera.imageSize().width, camera.imageSize().height);
        u_intrinsics = projectionClipFromCamera(
            camera.imageSize(), camera.focalLength(),
            camera.principalPoint(), near_far
        ).cast<float>();
        u_cam_from_world = cam_from_world.cast<float>().matrix();

        PANGO_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    }
private:
    const Shared<GlSlProgram> prog = GlSlProgram::Create({
        .sources = {{ .origin="/components/pango_opengl/shaders/main_image.glsl" }}
    });
    GlVertexArrayObject vao = {};
    const GlUniform<int> texture_unit = {"image"};
    const GlUniform<Eigen::Matrix4f> u_intrinsics = {"proj"};
    const GlUniform<Eigen::Matrix4f> u_cam_from_world = {"cam_from_world"};
    const GlUniform<Eigen::Vector2f> u_image_size = {"image_size"};
};

struct DrawLayerImpl : public DrawLayer {
    Eigen::Array3f debug_random_color;

    DrawLayerImpl(const DrawLayerImpl::Params& p)
        : name_(p.name),
        size_hint_(p.size_hint),
        near_far_(p.near_far),
        camera_(p.camera),
        camera_from_world_(p.camera_from_world),
        camera_limits_in_world_(p.camera_limits_in_world),
        handler_(p.handler),
        cam_from_world_(Eigen::Matrix4d::Identity()),
        intrinsic_k_(Eigen::Matrix4d::Identity()),
        non_linear_(),
        objects_(p.objects)
    {
        debug_random_color = (Eigen::Array3f::Random() + 1.0f) / 2.0;
    }

    std::string name() const override
    {
        return name_;
    }

    void setDefaultImageParams(const DrawnImage& im)
    {
        const auto imsize = im.image->imageSize();
        const double start_dist = std::max(imsize.width, imsize.height) / 2.0;

        if(!camera_) {
            camera_ = Shared<sophus::CameraModel>::make(
                sophus::createDefaultPinholeModel(imsize)
            );
        }

        if(!camera_from_world_) {
            camera_from_world_ = Shared<sophus::Se3F64>::make(
                cameraLookatFromWorld(
                    {imsize.width/2.0, imsize.height/2.0, -start_dist},
                    {imsize.width/2.0, imsize.height/2.0, 0.0} )
            );
        }

        if(near_far_.empty()) {
            near_far_ = MinMax<double>(0.1, 100.0*start_dist);
        }

        if(camera_limits_in_world_.empty()) {
            camera_limits_in_world_ = {
                {0.0, 0.0, -start_dist},
                {imsize.width, imsize.height, -0.2},
            };
        }
    }

    void setDefaultPrimitivesParams(const DrawnPrimitives& prim)
    {
        // TODO: based these off of the data in prim
        if(!camera_) {
            camera_ = Shared<sophus::CameraModel>::make(
                sophus::createDefaultPinholeModel({100,100})
            );
        }

        if(!camera_from_world_) {
            camera_from_world_ = Shared<sophus::Se3F64>::make(
                cameraLookatFromWorld( {0.0, 0.0, -5.0}, {0.0, 0.0, 0.0} )
            );
        }

        if(near_far_.empty()) {
            near_far_ = MinMax<double>(0.1, 1000.0);
        }
    }

    void renderIntoRegion(const Context& c, const RenderParams& p) override {
        ScopedGlEnable en_scissor(GL_SCISSOR_TEST);
        c.setViewport(p.region);
        glClearColor(debug_random_color[0], debug_random_color[2], debug_random_color[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for(auto& obj : objects_) {
            if(DrawnImage* im = dynamic_cast<DrawnImage*>(obj.ptr())) {
                if(!camera_ || !camera_from_world_) setDefaultImageParams(*im);
                PANGO_ENSURE(camera_ && camera_from_world_);
                render_image_.draw(*im, *camera_, *camera_from_world_, near_far_);
            }else if(DrawnPrimitives* prim = dynamic_cast<DrawnPrimitives*>(obj.ptr())) {
                if(!camera_ || !camera_from_world_) setDefaultPrimitivesParams(*prim);
                PANGO_ENSURE(camera_ && camera_from_world_);
                render_primitives_.draw(*prim, *camera_, *camera_from_world_, near_far_);
            }
        }
    }

    bool handleEvent(const Context& context, const Event& event) override {
        if(handler_ && camera_from_world_ && camera_) {
            handler_->handleEvent(
                *this, *camera_from_world_, *camera_, near_far_,
                camera_limits_in_world_,context, event
            );
            return true;
        }
        return false;
    }

    Size sizeHint() const override {
        return size_hint_;
    }

    void setHandler(const std::shared_ptr<Handler>& handler) override {
        handler_ = handler;
    }

    MinMax<Eigen::Vector3d> getSceneBoundsInWorld() const override {
        return bounds_;
    }

    void add(const Shared<Drawable>& r) override {
        objects_.push_back(r);
    }

    void remove(const Shared<Drawable>& r) override {
        throw std::runtime_error("Not implemented yet...");
    }

    void clear() override {
        objects_.clear();
    }

    std::string name_;
    Size size_hint_;
    MinMax<double> near_far_;
    MinMax<Eigen::Vector3d> bounds_;

    // Used for handling and higher-level logic
    std::shared_ptr<sophus::CameraModel> camera_;
    std::shared_ptr<sophus::Se3F64> camera_from_world_;
    MinMax<Eigen::Vector3d> camera_limits_in_world_ = {};
    std::shared_ptr<Handler> handler_;

    // Actually used for rendering
    Eigen::Matrix4d cam_from_world_;
    Eigen::Matrix4d intrinsic_k_;
    NonLinearMethod non_linear_;

    std::vector<Shared<Drawable>> objects_;
    DrawnImageProgram render_image_;
    DrawnPrimitivesProgram render_primitives_;
};

PANGO_CREATE(DrawLayer) {
    return Shared<DrawLayerImpl>::make(p);
}

}
