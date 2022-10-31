#include <fmt/format.h>

#include <pangolin/context/context.h>
#include <pangolin/context/factory.h>
#include <pangolin/windowing/window.h>
#include <pangolin/gui/render_layer_group.h>
#include <pangolin/utils/variant_overload.h>

namespace pangolin
{

namespace debug
{
    void print(const RenderLayerGroup& layout) {
        const auto& v = layout.children;
        FARM_CHECK(v.size() > 0);
        if(layout.layer) fmt::print("x");
        fmt::print("(");
        print(v[0]);
        for(size_t i=1; i < v.size(); ++i) {
            switch (layout.grouping)
            {
            case RenderLayerGroup::Grouping::horizontal: fmt::print("|"); break;
            case RenderLayerGroup::Grouping::vertical: fmt::print("/"); break;
            case RenderLayerGroup::Grouping::tabbed: fmt::print(","); break;
            case RenderLayerGroup::Grouping::stacked: fmt::print("^"); break;
            default: break;
            }
            print(v[i]);
        }
        fmt::print(")");
    }
}

struct EngineImpl : public Engine
{
};

Shared<Engine> Engine::singleton()
{
    static Shared<Engine> global = Shared<EngineImpl>::make();
    return global;
}

struct ContextImpl : public Context {
    // TODO: Convert Window to use new factory idiom directly
    ContextImpl(const Context::Params& params)
        : window_(Window::Create({
            .uri = ParseUri(
                fmt::format("{}:[window_title={},w={},h={}]//",
                 params.window_engine, params.title,
                 params.window_size.width, params.window_size.height)
            )
        })),
        size_(params.window_size)
    {
        window()->ResizeSignal.connect([this](const WindowResizeEvent& e){
            size_.width = e.width;
            size_.height = e.height;
        });
    }

    Shared<Window> window() override {
        return window_;
    }

    void setLayout(const RenderLayerGroup& layout) override
    {
        layout_ = layout;
    }

    void setLayout(const Shared<RenderLayer>& panel) override
    {
        setLayout(RenderLayerGroup(panel));
    }

    RenderLayerGroup getLayout() const override
    {
        return layout_;
    }

    void drawPanels()
    {
        MinMax<Eigen::Vector2i> region;
        region.extend({0,0});
        region.extend({size_.width, size_.height});

        renderIntoRegion({
            .region = region
        }, getLayout());
    }

    void loop(std::function<bool(void)> loop_function) override {
        bool should_run = true;

        auto close_connection = window()->CloseSignal.connect(
            [&](){ should_run = false; }
        );

        window()->MakeCurrent();

        while(should_run && loop_function()) {
            drawPanels();
            window()->SwapBuffers();
            window()->ProcessEvents();
        }
    }

    ImageSize size_;
    Shared<Window> window_;
    RenderLayerGroup layout_;
};

PANGO_CREATE(Context) {
    return Shared<ContextImpl>::make(p);
}


}
