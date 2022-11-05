#include <pangolin/gui/layer_group.h>

namespace pangolin
{

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

void computeLayoutRegion(const LayerGroup& group, const MinMax<Eigen::Array2i>& region) {
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
                const MinMax<Eigen::Array2i> child_region(
                    Eigen::Array2i(x, region.min()[1]),
                    Eigen::Array2i(x+w-1, region.max()[1])
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

            int y = region.min()[1];
            for(const auto& child : group.children) {
                const int h = int(child.cached_.min_pix[1] + child.cached_.parts[1] * ratio_unit_pix);
                const MinMax<Eigen::Array2i> child_region(
                    Eigen::Array2i(region.min()[0], y),
                    Eigen::Array2i(region.max()[0], y+h-1)
                );
                computeLayoutRegion(child, child_region);
                y += h + handle_pix;
            }
            // TODO: we'll have a couple of pixels left from rounding down
            break;
        }
    }
}

std::ostream& operator<<(std::ostream& s, const LayerGroup& layout) {
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
