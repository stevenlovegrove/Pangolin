#include <fmt/format.h>

#include <pangolin/context/context.h>
#include <pangolin/context/factory.h>
#include <pangolin/windowing/window.h>
#include <pangolin/gui/panel_group.h>
#include <pangolin/utils/variant_overload.h>

namespace pangolin
{

namespace debug
{
    void print(const Shared<Panel>& p) {
        fmt::print("x");
    }

    void print(const PanelGroup::Element& p) {
        std::visit([&](const auto& x){
            print(x);
        }, p);
    }

    void print(const Shared<PanelGroup>& layout) {
        const auto& v = layout->vec;
        FARM_CHECK(v.size() > 0);
        fmt::print("(");
        print(v[0]);
        for(size_t i=1; i < v.size(); ++i) {
            switch (layout->grouping)
            {
            case PanelGroup::Grouping::horizontal: fmt::print("|"); break;
            case PanelGroup::Grouping::vertical: fmt::print("/"); break;
            case PanelGroup::Grouping::tabbed: fmt::print(","); break;
            case PanelGroup::Grouping::stacked: fmt::print("^"); break;
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
                 params.window_size[0], params.window_size[1])
            )
        })),
        layout_(Shared<PanelGroup>::make())
    {
    }

    Shared<Window> window() override {
        return window_;
    }

    void setLayout(const Shared<PanelGroup>& layout) override
    {
        layout_ = layout;
    }

    void setLayout(const Shared<Panel>& panel) override
    {
        auto group = Shared<PanelGroup>::make();
        group->vec = {panel};
        setLayout(group);
    }

    Shared<PanelGroup> getLayout() const override
    {
        return layout_;
    }



    struct TSum {
        int pixels = 0;
        double parts = 0.0;
    };

    using TSum2 = Eigen::Vector<TSum,2>;

    TSum2 groupSum(const PanelGroup::Element& e)
    {
        TSum2 xy = std::visit(overload {
            [](const Shared<PanelGroup>& x) -> TSum2{
                return TSum2{};
            },
            [](const Shared<Panel>& x) -> TSum2 {
                //TODO need to be able to retrieve size_hint from x.
                return TSum2{};
            },
        }, e);

        return xy;
    }

    void drawPanels()
    {
        //  xy_sum;
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

    Shared<Window> window_;
    Shared<PanelGroup> layout_;
};

PANGO_CREATE(Context) {
    return Shared<ContextImpl>::make(p);
}


}
