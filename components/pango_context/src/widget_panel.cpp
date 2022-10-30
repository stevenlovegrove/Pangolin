#include <pangolin/context/factory.h>
#include <pangolin/gui/widget_panel.h>

namespace pangolin
{

struct PanelImpl : public RenderLayer {
    PanelImpl(const RenderLayer::Params& p) {}
    void renderIntoRegion(const RenderParams&){};
};

struct SeperatorImpl : public Seperator {
    SeperatorImpl(const SeperatorImpl::Params& p) {}
};

struct SliderImpl : public Slider {
    SliderImpl(const SliderImpl::Params& p) {}
};

struct WidgetPanelImpl : public WidgetPanel {
    WidgetPanelImpl(const WidgetPanelImpl::Params& p) {}
    void renderIntoRegion(const RenderParams&) override{
    }
    void add(SharedVector<Widget> widgets) override {

    };
};

PANGO_CREATE(RenderLayer) {
    return Shared<PanelImpl>::make(p);
}

PANGO_CREATE(Seperator) {
    return Shared<SeperatorImpl>::make(p);
}

PANGO_CREATE(Slider) {
    return Shared<SliderImpl>::make(p);
}

PANGO_CREATE(WidgetPanel) {
    return Shared<WidgetPanelImpl>::make(p);
}

}
