#include <pangolin/context/widget.h>
#include <pangolin/context/factory.h>

namespace pangolin
{

struct PanelImpl : public Panel {
    PanelImpl(const Panel::Params& p) {}
};

struct SeperatorImpl : public Seperator {
    SeperatorImpl(const SeperatorImpl::Params& p) {}
};

struct SliderImpl : public Slider {
    SliderImpl(const SliderImpl::Params& p) {}
};

struct WidgetPanelImpl : public WidgetPanel {
    WidgetPanelImpl(const WidgetPanelImpl::Params& p) {}
    
    void add(SharedVector<Widget> widgets) override {

    };
};

struct MultiPanelImpl : public MultiPanel {
    MultiPanelImpl(const MultiPanelImpl::Params& p) {}
};

PANGO_CREATE(Panel) {
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

PANGO_CREATE(MultiPanel) {
    return Shared<MultiPanelImpl>::make(p);
}

}