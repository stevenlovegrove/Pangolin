#pragma once

#include <algorithm>
#include <pangolin/gui/layer.h>
#include <pangolin/utils/variant_overload.h>
#include <pangolin/maths/eigen_scalar_methods.h>

namespace pangolin
{

////////////////////////////////////////////////////////////////////
/// Represents a (possibly nested) arrangement of Panels on screen
///
struct LayerGroup
{
    enum class Grouping
    {
        stacked,    // panes blended over one another
        tabbed,     // one pane shown at a time, with user selecting current
        horizontal, // panes share client area horizontally
        vertical    // panes share client area vertically
    };

    LayerGroup() = default;
    LayerGroup(Shared<Layer> layer) : layer(layer){}

    Grouping grouping = Grouping::horizontal;
    std::vector<LayerGroup> children = {};
    std::shared_ptr<Layer> layer = nullptr;

    struct LayoutInfo
    {
        Eigen::Array2i min_pix = {0,0};
        Eigen::Array2d parts = {0.0f, 0.0f};
        double width_over_height = 0.0;
        MinMax<Eigen::Array2i> region;
    };

    mutable LayoutInfo cached_;
};

////////////////////////////////////////////////////////////////////
// Define Convenience operators for building arrangements

// TODO: not sure this is worth it.
// void operator<(Shared<Context>& w, const Shared<LayerGroup>& g)
// {
//     w->setLayout(g);
// }

namespace detail
{
inline LayerGroup join( LayerGroup::Grouping op_type, const LayerGroup& lhs, const LayerGroup& rhs ) {
    LayerGroup ret;
    ret.grouping = op_type;

    if(op_type == lhs.grouping && op_type == rhs.grouping ) {
        ret.children = lhs.children;
        ret.children.insert(ret.children.end(), rhs.children.begin(), rhs.children.end());
    }else{
        ret.children.push_back(lhs);
        ret.children.push_back(rhs);
    }
    return ret;
}
template<DerivedFrom<Layer> T>
LayerGroup join( LayerGroup::Grouping op_type, const LayerGroup& lhs, const Shared<T>& rhs ) {
    LayerGroup ret;
    ret.grouping = op_type;
    if(op_type == lhs.grouping ) {
        ret.children = lhs.children;
    }else{
        ret.children.push_back(lhs);
    }
    ret.children.push_back(LayerGroup(rhs));
    return ret;
}
template<DerivedFrom<Layer> T>
LayerGroup join( LayerGroup::Grouping op_type, const Shared<T>& lhs, const LayerGroup& rhs ) {
    LayerGroup ret;
    ret.grouping = op_type;
    ret.children.push_back(LayerGroup(lhs));
    if(op_type == rhs.grouping) {
        ret.children.insert(ret.children.end(), rhs.children.begin(), rhs.children.end());
    }else{
        ret.children.push_back(rhs);
    }
    return ret;
}
template<DerivedFrom<Layer> LHS, DerivedFrom<Layer> RHS>
LayerGroup join( LayerGroup::Grouping op_type, const Shared<LHS>& lhs, const Shared<RHS>& rhs ) {
    LayerGroup ret;
    ret.grouping = op_type;
    ret.children.push_back(LayerGroup(lhs));
    ret.children.push_back(LayerGroup(rhs));
    return ret;
}
}

#define PANGO_PANEL_OPERATOR(op, op_type) \
    inline LayerGroup op(const LayerGroup& lhs, const LayerGroup& rhs) { \
        return detail::join(op_type, lhs, rhs); \
    } \
    template<DerivedFrom<Layer> T> \
    LayerGroup op(const LayerGroup& lhs, const Shared<T>& rhs) { \
        return detail::join(op_type, lhs, rhs); \
    } \
    template<DerivedFrom<Layer> T> \
    LayerGroup op(const Shared<T>& lhs, const LayerGroup& rhs) { \
        return detail::join(op_type, lhs, rhs); \
    } \
    template<DerivedFrom<Layer> T1, DerivedFrom<Layer> T2> \
    LayerGroup op(const Shared<T1>& lhs, const Shared<T2>& rhs)  { \
        return detail::join(op_type, lhs, rhs); \
    }

#define PANGO_COMMA ,
PANGO_PANEL_OPERATOR(operator PANGO_COMMA, LayerGroup::Grouping::tabbed)
PANGO_PANEL_OPERATOR(operator|, LayerGroup::Grouping::horizontal)
PANGO_PANEL_OPERATOR(operator/, LayerGroup::Grouping::vertical)
PANGO_PANEL_OPERATOR(operator^, LayerGroup::Grouping::stacked)
#undef PANGO_PANEL_OPERATOR
#undef PANGO_COMMA

void computeLayoutConstraints(const LayerGroup& group);
void computeLayoutRegion(const LayerGroup& group, const MinMax<Eigen::Array2i>& region);

std::ostream& operator<<(std::ostream& s, const LayerGroup& layout);

}
