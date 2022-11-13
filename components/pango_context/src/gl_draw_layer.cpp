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
#include <sophus/sensor/orthographic.h>

#include "gl_utils.h"

#include <unordered_map>

using namespace sophus;

namespace pangolin
{

constexpr GLenum toGlEnum (DrawnPrimitives::Type type) {
    switch(type) {
        case DrawnPrimitives::Type::points: return GL_POINTS;
        case DrawnPrimitives::Type::lines: return GL_LINES;
        case DrawnPrimitives::Type::line_strip: return GL_LINE_STRIP;
        case DrawnPrimitives::Type::line_loop: return GL_LINE_LOOP;
        case DrawnPrimitives::Type::triangles: return GL_TRIANGLES;
        case DrawnPrimitives::Type::triangle_strip: return GL_TRIANGLE_STRIP;
        default: return 0;
    }
}

Eigen::Matrix4d linearClipFromCamera(const sophus::CameraModel& camera, MinMax<double> near_far)
{
    using namespace sophus;

    return std::visit(overload{
        [&](const OrthographicModel& cam){
            const auto bb = boundingBoxFromOrthoCam(cam);
            const MinMax<Eigen::Vector2d> extent(bb.min(),bb.max());

            return projectionClipFromOrtho(
                extent, near_far,
                ImageXy::right_down,
                // already specified correctly in OrthographicModel's coords
                ImageIndexing::pixel_continuous
            );
        },
        [&](const auto& cam){
            if(camera.distortionType() != sophus::CameraDistortionType::pinhole) {
                PANGO_WARN("Ignoring distortion component of camera for OpenGL rendering for now.");
            }
            return projectionClipFromCamera(
                cam.imageSize(), cam.focalLength(),
                cam.principalPoint(), near_far
            );
        }
    }, camera.modelVariant());
}

Eigen::Matrix3d linearCameraFromImage(const sophus::CameraModel& camera)
{
    return invProjectionCameraFromImage(camera.focalLength(), camera.principalPoint());
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
            auto enable_depth = (primitives.enable_visibility_testing) ?
                ScopedGlDisable::noOp() : ScopedGlDisable(GL_DEPTH_TEST);

            u_intrinsics = linearClipFromCamera(camera, near_far).cast<float>();
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
    // This program renders an image on the z=0 plane in a discrete pixel
    // convention such that x=0,y=0 is the center of the top-left pixel.
    // As such the vertex extremes for the texture map from (-0.5,-0.5) to
    // (w-0.5,h-0.5).
    // TODO: Add a flag to take convention as configuration.

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
        u_intrinsics = linearClipFromCamera(camera, near_far).cast<float>();
        u_cam_from_world = cam_from_world.cast<float>().matrix();

        // TODO: load from DrawnImage param if specified.
        if(drawn_image.image->pixelType().num_channels == 1) {
            u_color_transform = (Eigen::Matrix4f() << 1,0,0,0,  1,0,0,0,  1,0,0,0, 0,0,0,1).finished();
        }else{
            u_color_transform = Eigen::Matrix4f::Identity();
        }

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
    const GlUniform<Eigen::Matrix4f> u_color_transform = {"color_transform"};
};

struct DrawnSolidsProgram
{
    void draw(
        const DrawnSolids& primitives,
        const sophus::CameraModel& camera,
        const sophus::Se3F64& cam_from_world,
        MinMax<double> near_far
    ){
        auto bind_prog = prog->bind();
        auto bind_vao = vao.bind();

        u_image_size = Eigen::Vector2f(camera.imageSize().width, camera.imageSize().height);
        u_kinv = linearCameraFromImage(camera).cast<float>();
        u_world_from_cam = cam_from_world.inverse().cast<float>().matrix();
        u_znear_zfar = Eigen::Vector2f(near_far.min(), near_far.max());

        PANGO_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    }

private:
    const Shared<GlSlProgram> prog = GlSlProgram::Create({
        .sources = {{ .origin="/components/pango_opengl/shaders/main_solids.glsl" }}
    });
    GlVertexArrayObject vao = {};
    const GlUniform<Eigen::Matrix3f> u_kinv = {"kinv"};
    const GlUniform<Eigen::Matrix4f> u_world_from_cam = {"world_from_cam"};
    const GlUniform<Eigen::Vector2f> u_image_size = {"image_size"};
    const GlUniform<Eigen::Vector2f> u_znear_zfar = {"znear_zfar"};
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

    bool setDefaultImageParams(const DrawnImage& im)
    {
        if(im.image->empty()) return false;

        const auto imsize = im.image->imageSize();

        if(!camera_) {
            auto ortho = CameraModel(imsize, CameraDistortionType::orthographic, Eigen::Vector4d{1.0,1.0, 0.0,0.0} );
            camera_ = Shared<CameraModel>::make(ortho);
        }

        if(!camera_from_world_) {
            camera_from_world_ = Shared<Se3F64>::make();
        }

        if(near_far_.empty()) {
            near_far_ = MinMax<double>(-1.0, 1.0);
        }

        if(camera_limits_in_world_.empty()) {
            camera_limits_in_world_ = {
                {0.0, 0.0, 0.0},
                {0.0, 0.0, 0.0},
            };
        }

        return true;
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

        for(auto& obj : objects_) {
            if(DrawnImage* im = dynamic_cast<DrawnImage*>(obj.ptr())) {
                if( (!camera_ || !camera_from_world_) && !setDefaultImageParams(*im)) continue;
                PANGO_ENSURE(camera_ && camera_from_world_);
                render_image_.draw(*im, *camera_, *camera_from_world_, near_far_);
            }else if(DrawnPrimitives* prim = dynamic_cast<DrawnPrimitives*>(obj.ptr())) {
                if(!camera_ || !camera_from_world_) setDefaultPrimitivesParams(*prim);
                PANGO_ENSURE(camera_ && camera_from_world_);
                render_primitives_.draw(*prim, *camera_, *camera_from_world_, near_far_);
            }else if(DrawnSolids* solids = dynamic_cast<DrawnSolids*>(obj.ptr())) {
                if(!camera_ || !camera_from_world_) setDefaultPrimitivesParams(*prim);
                PANGO_ENSURE(camera_ && camera_from_world_);
                render_solids_.draw(*solids, *camera_, *camera_from_world_, near_far_);
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

    void setCamera(std::shared_ptr<sophus::CameraModel>& camera) override {
        camera_ = camera;
    }

    void setCameraPoseFromWorld( std::shared_ptr<sophus::Se3F64>& camera_from_world) override {
        camera_from_world_ = camera_from_world;
    }

    std::shared_ptr<Handler> getHandler() const override {
        return handler_;
    }
    std::shared_ptr<sophus::CameraModel> getCamera() const override {
        return camera_;
    }
    std::shared_ptr<sophus::Se3F64> getCameraPoseFromWorld() const override {
        return camera_from_world_;
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
    DrawnSolidsProgram render_solids_;
};

PANGO_CREATE(DrawLayer) {
    return Shared<DrawLayerImpl>::make(p);
}

}
