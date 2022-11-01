#include <pangolin/context/factory.h>
#include <pangolin/gui/renderable.h>

namespace pangolin
{

// Right now, we have no specialization. Just instantiate the class itself
PANGO_CREATE(RenderableImage) {
    auto r = Shared<RenderableImage>::make();
    r->image->give(p.image, {});
    return r;
}

// Right now, we have no specialization. Just instantiate the class itself
PANGO_CREATE(RenderablePts) {
    return Shared<RenderablePts>::make();
}

}
