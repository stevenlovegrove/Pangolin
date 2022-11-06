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
    return Shared<DrawnPrimitives>::make();
}

}
