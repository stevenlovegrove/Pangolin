#include <pangolin/experimental/widget.h>

#define PANGO_CREATE(x) std::shared_ptr<x> x::Create(x::Params p)

namespace pangolin
{

struct WindowImpl : public Window
{
    void resize(const Size& window_size) override
    {
    }
};

struct ContextImpl : public Context
{
};


PANGO_CREATE(Context) {
    return std::shared_ptr<ContextImpl>(nullptr);
}

PANGO_CREATE(Panel) {
    return nullptr;
}

PANGO_CREATE(Seperator) {
    return nullptr;
}

PANGO_CREATE(Slider) {
    return nullptr;
}

PANGO_CREATE(WidgetPanel) {
    return nullptr;
}

PANGO_CREATE(MultiPanel) {
    return nullptr;
}

}

#undef PANGO_CREATE