#include <pangolin/context/factory.h>
#include <pangolin/gui/drawable.h>

namespace pangolin
{

// Right now, we have no specialization. Just instantiate the class itself
PANGO_CREATE(DrawnImage) {
    auto r = Shared<DrawnImage>::make();
    if(!p.image.isEmpty()) {
        r->image->update(p.image);
    }
    return r;
}

// Right now, we have no specialization. Just instantiate the class itself
PANGO_CREATE(DrawnPrimitives) {
    auto r = Shared<DrawnPrimitives>::make();
    r->element_type = p.element_type;
    r->world_from_drawable = p.world_from_drawable;
    r->default_color = p.default_color;
    r->default_radius = p.default_radius;
    r->enable_visibility_testing = p.enable_visibility_testing;
    return r;
}


// Right now, we have no specialization. Just instantiate the class itself
PANGO_CREATE(DrawnSolids) {
    auto r = Shared<DrawnSolids>::make();
    r->world_from_drawable = p.world_from_drawable;
    return r;
}

}
