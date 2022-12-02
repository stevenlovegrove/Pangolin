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
        handler_(DrawLayerHandler::Create({})),
        in_scene_(p.in_scene),
        in_pixels_(p.in_pixels)
    {
        render_state_.near_far = p.near_far;

        if(p.camera) {
            render_state_.camera = *p.camera;
        }

        if(p.camera_from_world) {
            render_state_.camera_from_world = *p.camera_from_world;
        }else{
            render_state_.camera_from_world = cameraLookatFromWorld( {0.0, 0.0, -5.0}, {0.0, 0.0, 0.0} );
        }
    }

    std::string name() const override
    {
        return name_;
    }

    const RenderState& renderState() const override {
        return render_state_;
    }

    void setCamera(const sophus::CameraModel& camera) override {
        render_state_.camera = camera;
    }

    void setCameraFromWorld(const sophus::Se3F64& cam_from_world) override {
        render_state_.camera_from_world = cam_from_world;
    }

    void setClipViewTransform(sophus::Sim2<double>& clip_view_transform) override {
        render_state_.clip_view_transform = clip_view_transform;
    }

    void setNearFarPlanes(const MinMax<double>& near_far) override {
        render_state_.near_far = near_far;
    }

    // // Shrinks viewport such that aspect matches the provided camera dimensions.
    // // Will leave padding on in dimension unrendered.
    // static MinMax<Eigen::Array2i>
    // viewportFromCameraAspect(double cam_aspect, MinMax<Eigen::Array2i> region)
    // {
    //     const Eigen::Array2i region_size = region.range();
    //     const double region_aspect = double(region_size.x()) / double(region_size.y());
    //     if(region_aspect > cam_aspect) {
    //         // keep height, change width
    //         const int new_width = int(cam_aspect * region_size.y());
    //         const int new_left = region.min().x() + int((region_size.x() - new_width) / 2.0);
    //         return {
    //             {new_left, region.min().y()},
    //             {new_left+new_width, region.max().y()}
    //         };
    //     }else{
    //         // keep width, change height
    //         const int new_height = int(region_size.x() / cam_aspect);
    //         const int new_bottom = region.min().y() + int((region_size.y() - new_height) / 2.0);
    //         return {
    //             {region.min().x(), new_bottom},
    //             {region.max().x(), new_bottom+new_height }
    //         };
    //     }
    // }

    std::optional<Eigen::Array2i> tryGetDrawableBaseImageSize() const
    {
        MinMax<Eigen::Vector3d> pixel_bounds;
        for(const auto& obj : in_pixels_) {
            pixel_bounds.extend(obj->boundsInParent());
        }
        const Eigen::Array2i dim = pixel_bounds.range().head<2>().cast<int>();
        if(0 < dim[0] && 0 < dim[1]) return dim;
        return std::nullopt;
    }

    bool tryInitializeEmptyCamera() {
        if(render_state_.camera.isEmpty()) {
            if(auto maybe_dim = tryGetDrawableBaseImageSize()) {
                sophus::ImageSize size = toImageSize(*maybe_dim);
                render_state_.camera = sophus::createDefaultPinholeModel(size);
                return true;
            }else if(in_scene_.size()) {
                render_state_.camera = sophus::createDefaultPinholeModel({640, 480});
                return true;
            }
            return false;
        }
        return true;
    }

    struct RenderData
    {
        Eigen::Array2d clip_aspect_scale = {1.0, 1.0};
        Eigen::Matrix4d clip_view = Eigen::Matrix4d::Identity();
        Eigen::Matrix4d clip_aspect = Eigen::Matrix4d::Identity();
        ViewParams pixel_params;
        ViewParams scene_params;
    };

    static Eigen::Matrix4d sim2To4x4(Sim2<double> sim2)
    {
        const Eigen::Matrix3d m3 = sim2.matrix();
        Eigen::Matrix4d ret = Eigen::Matrix4d::Identity();
        ret.topLeftCorner<2,2>() = m3.topLeftCorner<2,2>();
        ret.topRightCorner<2,1>() = m3.topRightCorner<2,1>();
        return ret;
    }

    static RenderData computeRenderData(
        const RenderState& render_state,
        AspectPolicy aspect_policy,
        MinMax<Eigen::Array2i> viewport
    ) {
        RenderData state;
        state.clip_view = sim2To4x4(render_state.clip_view_transform);
        state.clip_aspect_scale =
            axisScale(viewport.range(), toEigen(render_state.camera.imageSize()));

        if(aspect_policy == AspectPolicy::crop) {
            PANGO_UNIMPLEMENTED();
        }else{
            // We'll modify clip coords to fix aspect (extending field of view)
            state.clip_aspect = Eigen::Vector4d(
                    state.clip_aspect_scale.x(),
                    state.clip_aspect_scale.y(),
                    1.0, 1.0
                ).asDiagonal();
        }

        state.pixel_params = {
            .viewport = viewport,
            .camera_dim = render_state.camera.imageSize(),
            .near_far = {-2.0, 2.0},
            .camera_from_world = Eigen::Matrix4d::Identity(),
            .image_from_camera = Eigen::Matrix4d::Identity(),
            .clip_from_image = state.clip_view * state.clip_aspect *
                transformClipFromProjection(render_state.camera.imageSize()) *
                transformProjectionFromImage({-2.0,2.0}, GraphicsProjection::orthographic)
        };

        state.scene_params = {
            .viewport = viewport,
            .camera_dim = render_state.camera.imageSize(),
            .near_far = render_state.near_far,
            .camera_from_world = render_state.camera_from_world.matrix(),
            .image_from_camera = transformImageFromCamera4x4(render_state.camera),
            .clip_from_image = state.clip_view * state.clip_aspect *
                transformClipFromProjection(render_state.camera.imageSize()) *
                transformProjectionFromImage(render_state.near_far,
                    render_state.camera.distortionType() == sophus::CameraDistortionType::orthographic ?
                        GraphicsProjection::orthographic : GraphicsProjection::perspective
                )
        };
        return state;
    }

    void renderIntoRegion(const Context& context, const RenderParams& params) override {
        if(!tryInitializeEmptyCamera()) {
            return;
        }

        render_data_ = computeRenderData( render_state_, aspect_policy_, params.region );

        ScopedGlEnable en_scissor(GL_SCISSOR_TEST);

        ////////////////////////////////////////////////////////////////////////
        if(in_pixels_.size()) {
            context.setViewport(render_data_.pixel_params.viewport);
            for(auto& obj : in_pixels_) {
                obj->draw(render_data_.pixel_params);
            }

            if(in_scene_.size()) {
                // We'll clear depth buffer for use for in_scene_ Drawables
                glClear(GL_DEPTH_BUFFER_BIT);
            }
        }

        if(in_scene_.size()) {
            context.setViewport(render_data_.scene_params.viewport);
            for(auto& obj : in_scene_) {
                obj->draw(render_data_.scene_params);
            }
        }
    }

    bool handleEvent(const Context& context, const Event& event) override {
        Eigen::Matrix3d clip_from_window = transformWindowFromClip(event.pointer_pos.region).inverse();
        Eigen::Matrix3d pixel_from_window;
        // TODO: implement this...
        //  = (
        //     transformWindowFromClip(event.pointer_pos.region) *
        //     render_data_.clip_view *
        //     render_data_.clip_aspect *
        //     transformClipFromProjection(render_state_.camera.imageSize())
        // ).inverse();

        handler_->setViewMode( in_pixels_.size() ? DrawLayerHandler::ViewMode::image_plane : DrawLayerHandler::ViewMode::freeview);

        return handler_->handleEvent(
            context, event,
            clip_from_window,
            pixel_from_window,
            render_data_.clip_aspect_scale,
            *this, render_state_
        );
    }

    Size sizeHint() const override {
        return size_hint_;
    }

    double aspectHint() const override {
        MinMax<Eigen::Vector3d> cam_bounds;
        for(const auto& obj : in_pixels_) {
            cam_bounds.extend(obj->boundsInParent());
        }
        if(!cam_bounds.empty()) {
            Eigen::Vector2d dim = cam_bounds.range().head<2>();
            return dim.x() / dim.y();
        }
        return 0.0;
    };

    void add(const Shared<Drawable>& r, In domain, const std::string& name ) override {
        auto it = named_drawables_.find(name);
        if(!name.empty() && it != named_drawables_.end()) {
            // Name already exists so we'll remove the drawable it refers to. We
            // wont erase from map because we're just about to add to it again.
            remove(it->second);
        }

        switch(domain) {
        case In::scene:   in_scene_.push_back(r); break;
        case In::pixels: in_pixels_.push_back(r); break;
        default: PANGO_UNREACHABLE();
        }

        if(!name.empty()) {
            // add / replace named drawable.
            named_drawables_.emplace(name, r);
        }
    }

    std::shared_ptr<Drawable> get(const std::string& name) const override {
        auto it = named_drawables_.find(name);
        if( it != named_drawables_.end() ) {
            return it->second;
        }
        return nullptr;
    }

    bool remove(const std::shared_ptr<Drawable>& r) override {
        bool anything_erased = false;
        // auto scene_end = std::remove(in_scene_.begin(), in_scene_.end(), r);
        // anything_erased |= scene_end != in_scene_.end();
        // in_scene_.erase(scene_end, in_scene.end());

        // auto pixels_end = std::remove(in_pixels_.begin(), in_pixels_.end(), r);
        // anything_erased |= pixels_end != in_pixels_.end();
        // in_pixels_.erase(pixels_end, in_pixels_.end());

        return anything_erased;
    }

    void clear(std::optional<In> domain = std::nullopt) override {
        if(domain) {
            if(*domain == In::pixels) {
                in_pixels_.clear();
            }else if(*domain == In::scene) {
                in_scene_.clear();
            }
        }else{
            in_pixels_.clear();
            in_scene_.clear();
        }
    }

    std::string name_;
    Size size_hint_;
    AspectPolicy aspect_policy_;

    RenderState render_state_;
    RenderData render_data_;

    Shared<DrawLayerHandler> handler_;

    std::vector<Shared<Drawable>> in_scene_;
    std::vector<Shared<Drawable>> in_pixels_;
    std::map<std::string, Shared<Drawable>> named_drawables_;
};

PANGO_CREATE(DrawLayer) {
    return Shared<DrawLayerImpl>::make(p);
}

}
