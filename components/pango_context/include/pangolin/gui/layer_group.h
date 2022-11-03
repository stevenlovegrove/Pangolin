#pragma once

#include <algorithm>
#include <pangolin/context/context.h>
#include <pangolin/gui/layer.h>
#include <pangolin/utils/variant_overload.h>
#include <pangolin/maths/eigen_scalar_methods.h>
#include <pangolin/utils/reverse_iterable.h>

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
        Eigen::Vector2i min_pix = {0,0};
        Eigen::Vector2d parts = {0.0f, 0.0f};
        double width_over_height = 0.0;
        MinMax<Eigen::Vector2i> region;
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
LayerGroup join( LayerGroup::Grouping op_type, const LayerGroup& lhs, const LayerGroup& rhs ) {
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
    LayerGroup op(const LayerGroup& lhs, const LayerGroup& rhs) { \
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

void computeLayoutConstraints(const LayerGroup& group) {
    LayerGroup::LayoutInfo total = {};

    // 1st pass, bottom up
    for(const auto& child : group.children) {
        // ask child to compute its layout size and cache
        computeLayoutConstraints(child);
        auto& x_info = child.cached_;

        // Update our constraints base on those of the children
        switch(group.grouping) {
            case LayerGroup::Grouping::stacked:
                [[fallthrough]];
            case LayerGroup::Grouping::tabbed:
                total.min_pix = max(total.min_pix, x_info.min_pix);
                total.parts = max(total.parts, x_info.parts);
                break;
            case LayerGroup::Grouping::horizontal:
                total.min_pix[0] += x_info.min_pix[0];
                total.parts[0] += x_info.parts[0];
                total.min_pix[1] = max(total.min_pix[1], x_info.min_pix[1]);
                total.parts[1] = max(total.parts[1], x_info.parts[1]);
                break;
            case LayerGroup::Grouping::vertical:
                total.min_pix[1] += x_info.min_pix[1];
                total.parts[1] += x_info.parts[1];
                total.min_pix[0] = max(total.min_pix[0], x_info.min_pix[0]);
                total.parts[0] = max(total.parts[0], x_info.parts[0]);
                break;
        }
    }

    // Finally, apply any constraints from the group layer, if it exists
    if(group.layer) {
        for(int i=0; i < 2; ++i) {
            std::visit(overload {
                [&](Parts part)  {  total.parts[i] = max(total.parts[i], part.ratio); },
                [&](Pixels ab) {  total.min_pix[i] = max(total.min_pix[i], ab.pixels); },
            }, group.layer->sizeHint()[i]);
        }
    }

    // cache for later 2nd pass
    group.cached_ = total;
}

void computeLayoutRegion(const LayerGroup& group, const MinMax<Eigen::Vector2i>& region) {
    // 2nd pass, top down

    auto& r = group.cached_;
    r.region = region;

    constexpr int handle_pix = 5;
    const int num_children = group.children.size();
    const int total_handle_pix = handle_pix * (num_children-1);

    switch(group.grouping) {
        case LayerGroup::Grouping::stacked:
            [[fallthrough]];
        case LayerGroup::Grouping::tabbed:
            // the same region for stacked and tabbed.
            for(const auto& child : group.children) {
                computeLayoutRegion(child, region);
            }
            break;
        case LayerGroup::Grouping::horizontal:
        {
            // compute how many pixels we have to spend after accounting fixed stuff
            int remaining = region.range()[0] - r.min_pix[0] - total_handle_pix;

            // Compute the size of one ratio unit given total and pixels we have
            // to fill.
            const double ratio_unit_pix = remaining / r.parts[0];

            int x = region.min()[0];
            for(const auto& child : group.children) {
                const int w = int(child.cached_.min_pix[0] + child.cached_.parts[0] * ratio_unit_pix);
                const MinMax<Eigen::Vector2i> child_region(
                    Eigen::Vector2i(x, region.min()[1]),
                    Eigen::Vector2i(x+w-1, region.max()[1])
                );
                computeLayoutRegion(child, child_region);
                x += w + handle_pix;
            }
            // TODO: we'll have a couple of pixels left from rounding down
            break;
        }
        case LayerGroup::Grouping::vertical:
        {
            // compute how many pixels we have to spend after accounting fixed stuff
            int remaining = region.range()[1] - r.min_pix[1] - total_handle_pix;

            // Compute the size of one ratio unit given total and pixels we have
            // to fill.
            const double ratio_unit_pix = remaining / r.parts[1];

            // Iterate in reverse because y-min is at the bottom in window coordinates
            int y = region.min()[1];
            for(const auto& child : reverse(group.children)) {
                const int h = int(child.cached_.min_pix[1] + child.cached_.parts[1] * ratio_unit_pix);
                const MinMax<Eigen::Vector2i> child_region(
                    Eigen::Vector2i(region.min()[0], y),
                    Eigen::Vector2i(region.max()[0], y+h-1)
                );
                computeLayoutRegion(child, child_region);
                y += h + handle_pix;
            }
            // TODO: we'll have a couple of pixels left from rounding down
            break;
        }
    }
}

void renderIntoRegionImpl(
    const Layer::RenderParams& p,
    const LayerGroup& group
) {
    if(group.layer) {
        group.layer->renderIntoRegion({
            .region = group.cached_.region
        });
    }
    for(const auto& child : group.children) {
        renderIntoRegionImpl(p, child);
    }
}

void renderIntoRegion(
    const Layer::RenderParams& p,
    const LayerGroup& group
) {
    computeLayoutConstraints(group);
    computeLayoutRegion(group, p.region);
    renderIntoRegionImpl(p, group);
}

inline std::ostream& operator<<(std::ostream& s, const LayerGroup& layout) {
    const auto& v = layout.children;
    FARM_CHECK(v.size() > 0);
    if(layout.layer) s << "x";
    s << "(";
    s << v[0];
    for(size_t i=1; i < v.size(); ++i) {
        switch (layout.grouping)
        {
        case LayerGroup::Grouping::horizontal: s << "|"; break;
        case LayerGroup::Grouping::vertical:   s << "/"; break;
        case LayerGroup::Grouping::tabbed:     s << ","; break;
        case LayerGroup::Grouping::stacked:    s << "^"; break;
        default: break;
        }
        s << v[i];
    }
    s << ")";
    return s;
}

}
