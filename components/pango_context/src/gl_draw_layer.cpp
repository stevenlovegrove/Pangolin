#include <pangolin/gui/draw_layer.h>
#include <pangolin/context/context.h>
#include <pangolin/context/factory.h>
#include <pangolin/render/projection_lut.h>
#include <pangolin/gl/gl.h>
#include <pangolin/var/var.h>

#include "camera_utils.h"
#include "gl_utils.h"
#include "draw_layer_handler.h"

#include <unordered_map>

using namespace sophus;

namespace pangolin
{

struct DrawLayerImpl : public DrawLayer {
    DrawLayerImpl(const DrawLayerImpl::Params& p)
        : name_(p.name),
        size_hint_(p.size_hint),
        aspect_policy_(p.aspect_policy),
        // cam_from_world_(Eigen::Matrix4d::Identity()),
        // intrinsic_k_(Eigen::Matrix4d::Identity()),
        // non_linear_(),
        handler_(DrawLayerHandler::Create({})),
        objects_(p.objects),
        objects_in_camera_(p.objects_in_camera)
    {
        *view_params_ = p.view_params;
        *view_constraints_ = p.view_constraints;
    }

    std::string name() const override
    {
        return name_;
    }

    void setViewParams(std::shared_ptr<ViewParams>& view_params) override {
        if(view_params) {
            view_params_ = view_params;
        }else{
            PANGO_UNIMPLEMENTED("Reset to defaults");
        }
    }

    Shared<ViewParams> getViewParams() const override {
        return view_params_;
    }

    void setViewConstraints(std::shared_ptr<ViewConstraints>& view_constraints) override {
        if(view_constraints) {
            view_constraints_ = view_constraints;
        }else{
            PANGO_UNIMPLEMENTED("Reset to defaults");
        }
    }

    Shared<ViewConstraints> getViewConstraints() const override {
        return view_constraints_;
    }

    void setupDefaultCameraParams() {
        if( objects_.empty() ) {
            MinMax<Eigen::Vector3d> bounds_in_cam;
            for(const auto& obj : objects_in_camera_) {
                bounds_in_cam.extend(obj->boundsInParent());
            }
            // can use orthographic camera in this case
            if(bounds_in_cam.min().z() == 1.0 &&  bounds_in_cam.max().z() == 1.0)  {
                const Eigen::Vector2i dim = bounds_in_cam.range().head<2>().cast<int>();
                view_params_->camera = sophus::CameraModel(
                        ImageSize(dim.x(), dim.y()),
                        sophus::CameraDistortionType::orthographic,
                        Eigen::Vector4d{1.0,1.0, 0.0,0.0}
                    );
                view_params_->near_far = {-2.0, 2.0};
            }
        }else{
            view_params_->camera =
                sophus::createDefaultPinholeModel({640,480});
        }

    }

    void setupDefaultModelViewParams() {
        if( !objects_.empty() ) {
            // TODO: figure out something better
            view_params_->camera_from_world = cameraLookatFromWorld( {0.0, 0.0, -5.0}, {0.0, 0.0, 0.0} );
        }
    }


    // bool setDefaultImageParams(const DrawnImage& im)
    // {
    //     if(im.image->imageSize().isEmpty()) return false;

    //     const auto imsize = im.image->imageSize();

    //     if(!camera_) {
    //         auto ortho = CameraModel(imsize, CameraDistortionType::orthographic, Eigen::Vector4d{1.0,1.0, 0.0,0.0} );
    //         camera_ = Shared<CameraModel>::make(ortho);
    //     }

    //     if(!camera_from_world_) {
    //         camera_from_world_ = Shared<Se3F64>::make();
    //     }

    //     if(near_far_.empty()) {
    //         near_far_ = MinMax<double>(-1.0, 1.0);
    //     }

    //     if(handler_ && handler_->getCameraLimits().empty()) {
    //         handler_->setCameraLimits({
    //             {0.0, 0.0, 0.0},
    //             {0.0, 0.0, 0.0},
    //         });
    //         handler_->setCameraRotationLock(true);
    //     }

    //     return true;
    // }

    // Shrinks viewport such that aspect matches the provided camera dimensions.
    // Will leave padding on in dimension unrendered.
    static MinMax<Eigen::Array2i>
    viewportFromCamera(const ImageSize& camera_dim, MinMax<Eigen::Array2i> region)
    {
        const Eigen::Array2i region_size = region.range();
        const double cam_aspect = double(camera_dim.width) / double(camera_dim.height);
        const double region_aspect = double(region_size.x()) / double(region_size.y());
        if(region_aspect > cam_aspect) {
            // keep height, change width
            const int new_width = int(cam_aspect * region_size.y());
            const int new_left = region.min().x() + int((region_size.x() - new_width) / 2.0);
            return {
                {new_left, region.min().y()},
                {new_left+new_width, region.max().y()}
            };
        }else{
            // keep width, change height
            const int new_height = int(region_size.x() / cam_aspect);
            const int new_bottom = region.min().y() + int((region_size.y() - new_height) / 2.0);
            return {
                {region.min().x(), new_bottom},
                {region.max().x(), new_bottom+new_height }
            };
        }
    }

    CameraModel cameraFromViewport(MinMax<Eigen::Array2i> region, const CameraModel& camera)
    {
        const CameraDistortionType orig_distortion_type = camera.distortionType();
        const Eigen::VectorXd orig_params = camera.params();
        const ImageSize orig_camera_dim = camera.imageSize();
        const Eigen::Vector2d orig_pp = camera.principalPoint();

        const Eigen::Array2i region_size = region.range();
        const double cam_aspect = double(orig_camera_dim.width) / double(orig_camera_dim.height);
        const double region_aspect = double(region_size.x()) / double(region_size.y());
        if(cam_aspect > region_aspect) {
            // need to increase camera height and y-principle point
            const int new_cam_height = int(orig_camera_dim.width / region_aspect);
            Eigen::Vector2d new_pp = orig_pp + Eigen::Vector2d(0.0, (new_cam_height - orig_camera_dim.height) / 2.0);
            CameraModel new_cam({orig_camera_dim.width, new_cam_height}, orig_distortion_type, orig_params);
            new_cam.setPrincipalPoint(new_pp);
            return new_cam;
        }else{
            // need to increase camera width and x-principle point
            const int new_cam_width = int(region_aspect * orig_camera_dim.height);
            Eigen::Vector2d new_pp = orig_pp + Eigen::Vector2d((new_cam_width - orig_camera_dim.width) / 2.0, 0.0);
            CameraModel new_cam({new_cam_width, orig_camera_dim.height}, orig_distortion_type, orig_params);
            new_cam.setPrincipalPoint(new_pp);
            return new_cam;
        }
    }

    void renderIntoRegion(const Context& context, const RenderParams& params) override {
        // Try to initialize view params automatically if none are provided.
        if(view_params_->camera.imageSize().width == 0) {
            setupDefaultCameraParams();
        }
        if( (view_params_->camera_from_world.params() - SE3d().params()).norm() < sophus::kEpsilon<double> ) {
            setupDefaultModelViewParams();
        }

        aspect_viewport_ = viewportFromCamera(view_params_->camera.imageSize(), params.region);

        // defaults for this render
        MinMax<Eigen::Array2i> viewport = (aspect_policy_ == AspectPolicy::crop) ?
            aspect_viewport_ : params.region;

        ViewParams view_params = *view_params_;
        if(aspect_policy_ == AspectPolicy::mask) {
            view_params.camera = cameraFromViewport(params.region, view_params.camera);
        }

        ScopedGlEnable en_scissor(GL_SCISSOR_TEST);
        context.setViewport(viewport);
        const Eigen::Vector2i viewport_dim = viewport.range();

        // Render camera frame objects
        for(auto& obj : objects_in_camera_) {
            ViewParams cam_view_params = view_params;
            cam_view_params.camera_from_world = SE3d();
            obj->draw(cam_view_params, viewport_dim);
        }

        // bit of a hack, but we'll assume camera frame stuff above is mostly 2d
        // drawing which we'll use as a background for 3d drawing.
        if(objects_in_camera_.size()) {
            glClear(GL_DEPTH_BUFFER_BIT);
        }

        // Render 3D scene
        for(auto& obj : objects_) {
            obj->draw(view_params, viewport_dim);
        }
    }

    bool handleEvent(const Context& context, const Event& event) override {
        Event viewport_event = event;
        // This is the aspect maintained region
        viewport_event.pointer_pos.region_ = aspect_viewport_;
        return handler_->handleEvent( context, viewport_event, *this);
    }

    Size sizeHint() const override {
        return size_hint_;
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
    AspectPolicy aspect_policy_;

    Shared<ViewParams> view_params_;
    Shared<ViewConstraints> view_constraints_;
    Shared<DrawLayerHandler> handler_;

    // // Actually used for rendering
    // Eigen::Matrix4d cam_from_world_;
    // Eigen::Matrix4d intrinsic_k_;
    // NonLinearMethod non_linear_;

    std::vector<Shared<Drawable>> objects_;
    std::vector<Shared<Drawable>> objects_in_camera_;

    mutable MinMax<Eigen::Array2i> aspect_viewport_;
};

PANGO_CREATE(DrawLayer) {
    return Shared<DrawLayerImpl>::make(p);
}

}
