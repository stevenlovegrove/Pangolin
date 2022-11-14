#include <fmt/format.h>

#include <pangolin/context/context.h>
#include <pangolin/context/factory.h>
#include <pangolin/windowing/window.h>
#include <pangolin/gui/layer_group.h>
#include <pangolin/utils/variant_overload.h>
#include <pangolin/utils/reverse_iterable.h>
#include <pangolin/handler/interactive.h>
#include <pangolin/gl/glplatform.h>
#include <pangolin/gl/gl_type_info.h>

namespace pangolin
{

struct EngineImpl : public Engine
{
};

Shared<Engine> Engine::singleton()
{
    static Shared<Engine> global = Shared<EngineImpl>::make();
    return global;
}

void renderIntoRegionImpl(
    const Context& context,
    const Layer::RenderParams& p,
    const LayerGroup& group
) {
    if(group.layer) {
        group.layer->renderIntoRegion(context, {
            .region = group.cached_.region
        });
    }
    if(group.grouping == LayerGroup::Grouping::stacked) {
        // Back-to-front for blending / overlays
        for(const auto& child : reverse(group.children)) {
            renderIntoRegionImpl(context, p, child);
        }
    }else if(group.grouping == LayerGroup::Grouping::tabbed) {
        PANGO_ENSURE(group.selected_tab < group.children.size());
        renderIntoRegionImpl(context, p, group.children[group.selected_tab]);
    }else{
        for(const auto& child : group.children) {
            renderIntoRegionImpl(context, p, child);
        }
    }
}

void renderIntoRegion(
    const Context& context,
    const Layer::RenderParams& p,
    const LayerGroup& group
) {
    computeLayoutConstraints(group);
    computeLayoutRegion(group, p.region);
    renderIntoRegionImpl(context, p, group);
}

// Find layer to receive event based on pointer location
// Return the layer which processed the event.
//
// Pre-condition: layer positions have been computed by a previous call to render
std::shared_ptr<Layer> giveEventToLayers(
    const Context& context,
    Interactive::Event event,
    const LayerGroup& group
) {
    const auto r = group.cached_.region;
    const Eigen::Vector2i winpos =
        event.pointer_pos.posInWindow().cast<int>();

    if(r.contains(winpos))
    {
        event.pointer_pos.region_ = r;

        if(group.layer && group.layer->handleEvent(context, event)) {
            // event handled, stop dfs
            return group.layer;
        }

        // see if child nodes want it
        for(const auto& child : group.children) {
            auto layer = giveEventToLayers(context, event, child);
            if(layer) return layer;
        }
    }
    return nullptr;
}

// This is kinda dumb - we look through all nodes even though
// we know the active_layer already. We do this just so we can
// find the cached region so we can send it through
bool giveEventToActiveLayer(
    const Context& context,
    Interactive::Event event,
    const LayerGroup& group,
    const std::shared_ptr<Layer>& active_layer
) {
    PANGO_ENSURE(active_layer);

    const auto r = group.cached_.region;
    event.pointer_pos.region_ = r;

    if(group.layer == active_layer) {
        group.layer->handleEvent(context, event);
    }

    // see if child nodes want it
    for(const auto& child : group.children) {
        const bool found = giveEventToActiveLayer(context, event, child, active_layer);
        if(found) return true;
    }

    return false;
}

struct ContextImpl : public Context {
    // TODO: Convert Window to use new factory idiom directly
    ContextImpl(const Context::Params& params)
        : size_(params.window_size),
        window_(Window::Create({
            .uri = ParseUri(
                fmt::format("{}:[window_title={},w={},h={},GL_PROFILE={}]//",
                 params.window_engine, params.title,
                 params.window_size.width, params.window_size.height, "3.2 CORE")
            )
        }))
    {
        window()->ResizeSignal.connect([this](const WindowResizeEvent& e){
            size_.width = e.width;
            size_.height = e.height;
            // commenting these in will trigger render during
            // resize, but there can be artifacts
            // window()->SwapBuffers();
            // drawPanels();
        });
        window()->MouseSignal.connect(&ContextImpl::mouseEvent, this);
        window()->MouseMotionSignal.connect(&ContextImpl::mouseMotionEvent, this);
        window()->SpecialInputSignal.connect(&ContextImpl::specialInputEvent, this);
        window()->KeyboardSignal.connect(&ContextImpl::keyboardEvent, this);

        window()->MakeCurrent();
        window()->ProcessEvents();
        glInit();
    }

    void glInit()
    {
        // All functions will maintain these as an invariant on return.
        glewInit();
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable( GL_BLEND );
    }

    void setViewport(
        const MinMax<Eigen::Array2i>& bounds,
        ImageXy image_convention = Conventions::global().image_xy
    ) const override {
        const auto gl_bounds = regionGlFromConvention(bounds, image_convention);
        const Eigen::Array2i pos = gl_bounds.min();
        const Eigen::Array2i size = gl_bounds.range() + Eigen::Array2i(1,1);
        glViewport( pos.x(), pos.y(), size.x(), size.y() );
        glScissor( pos.x(), pos.y(), size.x(), size.y() );
    }

    void mouseEvent(MouseEvent e) {
        if(e.button == MouseWheelUp || e.button == MouseWheelDown) {
            const float delta = (e.button == MouseWheelDown ? 1.0f : -1.0f);
            Interactive::Event layer_event = {
                .pointer_pos = WindowPosition {.pos_window_ = {e.x,e.y}},
                .detail = Interactive::ScrollEvent {
                    .pan = Eigen::Array2d(0.0, delta)
                }
            };
            dispatchLayerEvent(layer_event);
        }else{
            Interactive::Event layer_event = {
                .pointer_pos = WindowPosition {.pos_window_ = {e.x,e.y}},
                .detail = Interactive::PointerEvent {
                    .action = e.pressed ? PointerAction::down : PointerAction::click_up,
                    // .button = e.button
                }
            };
            dispatchLayerEvent(layer_event, e.pressed ? ActiveLayerAction::capture: ActiveLayerAction::release);
        }
    }

    void mouseMotionEvent(MouseMotionEvent e) {
        Interactive::Event layer_event = {
            .pointer_pos = WindowPosition {.pos_window_ = {e.x,e.y}},
            .detail = Interactive::PointerEvent {
                .action = PointerAction::drag,
                // .button = e.button
            }
        };
        dispatchLayerEvent(layer_event);
    }

    void specialInputEvent(SpecialInputEvent e) {
        if( e.inType == InputSpecialScroll) {
            Interactive::Event layer_event = {
                .pointer_pos = WindowPosition {.pos_window_ = {e.x,e.y}},
                .detail = Interactive::ScrollEvent {
                    .pan = {e.p[0], e.p[1]}
                }
            };
            dispatchLayerEvent(layer_event);
        }
    }

    void keyboardEvent(KeyboardEvent e) {
        if(e.key == 27) { // escape
            should_run = false;
        }else if(e.key == 9) {  // tab
            window_->ShowFullscreen(TrueFalseToggle::Toggle);
        }else{
            PANGO_INFO("Unprocessed keypress, {}", int(e.key) );
        }
    }

    enum class ActiveLayerAction
    {
        ignore,
        capture,
        release
    };

    void dispatchLayerEvent(
        const Interactive::Event& src,
        ActiveLayerAction active_layer_action = ActiveLayerAction::ignore
    ) {
        if(active_layer_) {
            giveEventToActiveLayer(*this, src, layout_, active_layer_);
            if(active_layer_action == ActiveLayerAction::release) {
                active_layer_ = nullptr;
            }
        }else{
            auto layer = giveEventToLayers(*this, src, layout_);
            if(active_layer_action == ActiveLayerAction::capture) {
                active_layer_ = layer;
            }
        }
    }

    Shared<Window> window() override {
        return window_;
    }

    void setLayout(const LayerGroup& layout) override
    {
        layout_ = layout;
    }

    const LayerGroup& layout() const override
    {
        return layout_;
    }

    LayerGroup& layout() override
    {
        return layout_;
    }

    ImageSize size() const override
    {
        return size_;
    }


    void drawPanels()
    {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        MinMax<Eigen::Array2i> region;
        region.extend({0,0});
        region.extend({size_.width, size_.height});

        renderIntoRegion(*this, {
            .region = region
        }, layout_);
    }

    void loop(std::function<bool(void)> loop_function) override {
        should_run = true;

        auto close_connection = window()->CloseSignal.connect(
            [&](){ should_run = false; }
        );

        while(should_run && loop_function()) {
            drawPanels();
            window()->SwapBuffers();
            window()->ProcessEvents();
        }
    }

    sophus::IntensityImage<> read(
        MinMax<Eigen::Array2i> bounds,
        Attachment attachment,
        ImageXy image_axis_convention
    ) const override {
        using namespace sophus;
        const auto gl_bounds = regionGlFromConvention(bounds, image_axis_convention);
        const Eigen::Array2i imsize = bounds.range() + Eigen::Array2i(1,1);
        const bool is_depth = attachment == Attachment::depth;

        const RuntimePixelType pixel_type =  is_depth ?
            RuntimePixelType::fromTemplate<float>() :
            RuntimePixelType::fromTemplate<sophus::Pixel3U8>();

        const GlFormatInfo gl_pixel_type = glTypeInfo(pixel_type);

        sophus::IntensityImage<> image(ImageSize(imsize[0], imsize[1]), pixel_type);

        glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
        glDrawBuffer(GL_FRONT);

        glReadPixels(
            gl_bounds.min().x(), gl_bounds.min().y(),
            imsize.x(), imsize.y(),
            is_depth ? GL_DEPTH_COMPONENT : gl_pixel_type.gl_base_format,
            gl_pixel_type.gl_type,
            const_cast<uint8_t*>(image.rawPtr())
            );
        return image;
    }

private:
    MinMax<Eigen::Array2i> regionGlFromConvention(
        MinMax<Eigen::Array2i> bounds,
        ImageXy axis_convention
    ) const {
        if(axis_convention == ImageXy::right_up) {
            // Same as OpenGL
            return bounds;
        }else if(axis_convention == ImageXy::right_down) {
            // Reverse image Y
            const Eigen::Array2i pos = bounds.min();
            const Eigen::Array2i size = bounds.range() + Eigen::Array2i(1,1);
            int y = size_.height - pos.y() - size.y();
            return {{pos.x(), y}, {pos.x() + size.x()-1, y + size.y()-1}};
        }else{
            PANGO_UNREACHABLE();
        }
    }

    ImageSize size_;
    Shared<Window> window_;
    LayerGroup layout_;
    std::shared_ptr<Layer> active_layer_;
    std::atomic<bool> should_run;
};

PANGO_CREATE(Context) {
    return Shared<ContextImpl>::make(p);
}


}
