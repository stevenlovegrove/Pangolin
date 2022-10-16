#include <pangolin/context/context.h>
#include <pangolin/context/widget.h>

#define PANGO_CREATE(x) ExpectShared<x> x::Create(x::Params p)

namespace pangolin
{

struct WindowImpl : public Window
{
    void resize(const Size& window_size) override
    {
    }
};

struct ContextImpl : public Context {
    ContextImpl()
        : window_(Shared<WindowImpl>::make())
    {
        
    }

    Shared<Window> window() override {
        return window_;
    }

    bool loop(std::function<bool(void)> loop_function) override {
        return true;
    }

    Shared<Window> window_;
};


struct PanelImpl : public Panel {};
struct SeperatorImpl : public Seperator {};
struct SliderImpl : public Slider {};
struct WidgetPanelImpl : public WidgetPanel {
    void add(SharedVector<Widget> widgets) override {

    };
};
struct MultiPanelImpl : public MultiPanel {};

PANGO_CREATE(Context) {
    return Shared<ContextImpl>::make();
}

PANGO_CREATE(Panel) {
    return Shared<PanelImpl>::make();
}

PANGO_CREATE(Seperator) {
    return Shared<SeperatorImpl>::make();
}

PANGO_CREATE(Slider) {
    return Shared<SliderImpl>::make();
}

PANGO_CREATE(WidgetPanel) {
    return Shared<WidgetPanelImpl>::make();
}

PANGO_CREATE(MultiPanel) {
    return Shared<MultiPanelImpl>::make();
}

}

#undef PANGO_CREATE