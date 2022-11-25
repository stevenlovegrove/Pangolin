#pragma once

#include <pangolin/maths/camera_look_at.h>
#include <pangolin/maths/min_max.h>
#include <pangolin/gui/layer_group.h>
#include <sophus/sensor/camera_model.h>
#include <sophus/lie/se3.h>
#include <sophus/lie/sim2.h>

#include <variant>

namespace pangolin
{

template<typename T>
struct DrawableConversionTraits;

////////////////////////////////////////////////////////////////////
/// Supports displaying interactive 2D / 3D elements
///
struct DrawLayer : public Layer
{
    struct RenderState
    {
        sophus::CameraModel camera;
        sophus::SE3d camera_from_world;
        sophus::Sim2<double> clip_view_transform;
        MinMax<double> near_far;
    };

    struct ViewParams {
        MinMax<Eigen::Array2i> viewport;
        sophus::ImageSize camera_dim;
        MinMax<double> near_far;
        Eigen::Matrix4d camera_from_world;
        Eigen::Matrix4d image_from_camera;
        Eigen::Matrix4d clip_from_image;
    };

    struct Drawable {
        virtual ~Drawable() {}
        virtual void draw(const ViewParams&) = 0;
        virtual MinMax<Eigen::Vector3d> boundsInParent() const = 0;
        Eigen::Matrix4d parent_from_drawable = Eigen::Matrix4d::Identity();
    };

    enum In {
        scene,
        pixels
    };

    enum AspectPolicy {
        stretch,
        crop,
        overdraw,
        mask
    };

    virtual const RenderState& renderState() const = 0;
    virtual void setCamera(const sophus::CameraModel&) = 0;
    virtual void setCameraFromWorld(const sophus::Se3<double>&) = 0;
    virtual void setClipViewTransform(sophus::Sim2<double>&) = 0;
    virtual void setNearFarPlanes(const MinMax<double>&) = 0;

    virtual void add(const Shared<Drawable>& r, In domain, const std::string& name = "") = 0;
    virtual std::shared_ptr<Drawable> get(const std::string& name) const = 0;
    virtual void remove(const Shared<Drawable>& r) = 0;
    virtual void clear(std::optional<In> domain = std::nullopt) = 0;

    // Convenience method to add several drawables together
    template<typename ...Ts>
    void addInScene(const Ts&... ts) {
        (add(DrawableConversionTraits<Ts>::makeDrawable(ts), In::scene), ...);
    }

    // Convenience method to add several drawables together
    template<typename ...Ts>
    void addInPixels(const Ts&... ts) {
        (add(DrawableConversionTraits<Ts>::makeDrawable(ts), In::pixels), ...);
    }

    struct Params {
        std::string name = "";
        Size size_hint = {Parts{1}, Parts{1}};
        AspectPolicy aspect_policy = AspectPolicy::mask;

        std::optional<sophus::CameraModel> camera;
        std::optional<sophus::Se3F64> camera_from_world;
        MinMax<double> near_far = {1e-3, 1e6};

        // Objects to draw through modelview transform
        std::vector<Shared<Drawable>> in_scene = {};

        // Objects to draw in camera-frame, including 2D image-plane drawing.
        // These are 'camera pixels', not 'screen pixels'.
        std::vector<Shared<Drawable>> in_pixels = {};
    };
    static Shared<DrawLayer> Create(Params p);
};

////////////////////////////////////////////////////////////////////////
// Traits for extending what types can be converted to drawables

template<DerivedFrom<DrawLayer::Drawable> L>
struct DrawableConversionTraits<Shared<L>> {
static Shared<DrawLayer::Drawable> makeDrawable(const Shared<L>& x) {
    return x;
}};

template<DerivedFrom<DrawLayer::Drawable> L>
struct DrawableConversionTraits<std::shared_ptr<L>> {
static Shared<DrawLayer::Drawable> makeDrawable(const std::shared_ptr<L>& x) {
    return x;
}};

template<typename T>
concept DrawableConvertable = requires (T x) {
    {DrawableConversionTraits<T>::makeDrawable(x)} -> SameAs<Shared<DrawLayer::Drawable>>;
};

template<DrawableConvertable T>
Shared<DrawLayer::Drawable> makeDrawable(const T& v) {
    return DrawableConversionTraits<T>::makeDrawable(v);
}

////////////////////////////////////////////////////////////////////////
// Direct conversion of anything drawable to a layer

// Helper for adding Drawables directly into layouts
template<DerivedFrom<DrawLayer::Drawable> T>
struct LayerConversionTraits<Shared<T>> {
    static Shared<Layer> makeLayer(const Shared<T>& drawable) {
        return DrawLayer::Create({ .in_scene = {makeDrawable(drawable)} });
    }
};

}
