#include <pangolin/context/factory.h>
#include <pangolin/gui/drawable.h>

namespace pangolin
{

// Right now, we have no specialization. Just instantiate the class itself
PANGO_CREATE(DrawnImage) {
    auto r = Shared<DrawnImage>::make();
    r->image->update(p.image);
    return r;
}

// Right now, we have no specialization. Just instantiate the class itself
PANGO_CREATE(DrawnPrimitives) {
    auto r = Shared<DrawnPrimitives>::make();
    r->element_type = p.element_type;
    r->default_color = p.default_color;
    r->default_radius = p.default_radius;
    r->world_from_drawable = p.world_from_drawable;
    return r;
}

}
