#pragma once

#include <algorithm>
#include <pangolin/gui/widget_layer.h>
#include <pangolin/gui/draw_layer.h>
#include <pangolin/utils/variant_overload.h>
#include <pangolin/maths/eigen_scalar_methods.h>
#include <pangolin/utils/logging.h>
#include <pangolin/var/var.h>

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

    // return true if this layer and its children should be rendered
    inline bool isShown() const {return show_;}

    // show or hide this whole layer
    inline void show(bool visible=true) {
        show_ = visible;
    }

    // show or hide a layer in this LayerGroup tree
    inline void show(std::shared_ptr<Layer>& layer_to_show, bool visible=true) {
        if(layer == layer_to_show) {
            show(visible);
        }else{
            for(auto& lg : children) {
                lg.show(layer_to_show, visible);
            }
        }
    }

    Grouping grouping = Grouping::horizontal;
    std::vector<LayerGroup> children = {};
    std::shared_ptr<Layer> layer = nullptr;
    size_t selected_tab = 0;
    bool show_ = true;

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

// Trivial implementation of IntoLayerGroup for the group itself
// Overload this for other types which can be automatically placed into a LayerGroup
inline LayerGroup IntoLayerGroup(const LayerGroup& group) {
    return group;
}

// A layer is trivially wrapped into a LayerGroup
inline LayerGroup IntoLayerGroup(const Shared<Layer>& layer) {
    return {layer};
}

// Build a DrawLayer for Drawables specified directly.
// All defaults will be used for the DrawLayer
inline LayerGroup IntoLayerGroup(const Shared<Drawable>& drawable) {
    return IntoLayerGroup(
        DrawLayer::Create({
            .objects = {drawable}
        })
    );
}

// Allow images to be specified directly for convenience
inline LayerGroup IntoLayerGroup(const sophus::IntensityImage<>& image) {
    return IntoLayerGroup(
        DrawnImage::Create({image})
    );
}

// Allow Vars to be specified directly for convenience
template<typename T>
inline LayerGroup IntoLayerGroup(const Var<T>& var) {
    // TODO: not obvious what size we should pick...
    return IntoLayerGroup(WidgetLayer::Create({
        .name=var.Meta().full_name,
        .size_hint={Parts{1},Pixels{50}}
    }));
}

// Specialize
template<typename T>
concept Layoutable = requires (T x) {
        {IntoLayerGroup(x)} -> SameAs<LayerGroup>;
        };

namespace detail
{
inline LayerGroup join( LayerGroup::Grouping op_type, const LayerGroup& lhs, const LayerGroup& rhs ) {
    PANGO_ASSERT(lhs.children.size() > 0 || lhs.layer);
    PANGO_ASSERT(rhs.children.size() > 0 || rhs.layer);

    LayerGroup ret;
    ret.grouping = op_type;

    if( op_type == lhs.grouping && !lhs.layer) {
        // We can merge hierarchy
        ret.children = lhs.children;
    }else{
        ret.children.push_back(lhs);
    }

    if( op_type == rhs.grouping && !rhs.layer) {
        // We can merge hierarchy
        ret.children.insert(ret.children.end(), rhs.children.begin(), rhs.children.end());
    }else{
        ret.children.push_back(rhs);
    }

    return ret;
}
}

#define PANGO_PANEL_OPERATOR(op, op_type) \
    template<Layoutable LHS, Layoutable RHS> \
    LayerGroup op(const LHS& lhs, const RHS& rhs) { \
        return detail::join(op_type, IntoLayerGroup(lhs), IntoLayerGroup(rhs)); \
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
