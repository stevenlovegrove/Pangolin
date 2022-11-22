#pragma once

#include <pangolin/gui/layer_group.h>

#include <sophus/image/runtime_image.h>
#include <pangolin/utils/variant_overload.h>
#include <pangolin/maths/eigen_scalar_methods.h>
#include <pangolin/var/var.h>

namespace pangolin
{

////////////////////////////////////////////////////////////////////
// LayerTraits is a utility that can be specialized for different types to
// support direct addition to context layout using the layout operators
template<typename T>
struct LayerTraits;

template<> struct LayerTraits<LayerGroup> {
static LayerGroup toGroup(const LayerGroup& group) {
    return group;
}};

template<DerivedFrom<Layer> L> struct LayerTraits<Shared<L>> {
static LayerGroup toGroup(const Shared<L>& layer) {
    return {layer};
}};

template<DerivedFrom<Layer> L> struct LayerTraits<std::shared_ptr<L>> {
static LayerGroup toGroup(const std::shared_ptr<L>& layer) {
    return {layer};
}};

////////////////////////////////////////////////////////////////////

// Concept to accept types where the LayerTraits specialization has been defined
template<typename T>
concept Layoutable = requires (T x) {
        {LayerTraits<T>::toGroup(x)} -> SameAs<LayerGroup>;
        };


namespace detail {
// Implementation of how layergroups are composed (and simplified)
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

////////////////////////////////////////////////////////////////////
// Define syntactic sugar for composing layergroups to make layouts

#define PANGO_PANEL_OPERATOR(op, op_type) \
    template<Layoutable LHS, Layoutable RHS> \
    LayerGroup op(const LHS& lhs, const RHS& rhs) { \
        return detail::join( op_type, \
            LayerTraits<LHS>::toGroup(lhs), \
            LayerTraits<RHS>::toGroup(rhs)); \
    }

#define PANGO_COMMA ,
PANGO_PANEL_OPERATOR(operator PANGO_COMMA, LayerGroup::Grouping::tabbed)
PANGO_PANEL_OPERATOR(operator|, LayerGroup::Grouping::horizontal)
PANGO_PANEL_OPERATOR(operator/, LayerGroup::Grouping::vertical)
PANGO_PANEL_OPERATOR(operator^, LayerGroup::Grouping::stacked)
#undef PANGO_PANEL_OPERATOR
#undef PANGO_COMMA

template<typename T>
LayerGroup flex(T head)
{
    LayerGroup g;
    g.grouping = LayerGroup::Grouping::flex;
    g.children.push_back(LayerTraits<T>::toGroup(head));
    return g;
}

template<typename T, typename ...TArgs>
LayerGroup flex(T head, TArgs... args)
{
    return detail::join(
        LayerGroup::Grouping::flex,
        LayerTraits<T>::toGroup(head),
        flex(std::forward<TArgs>(args)...)
    );
}

////////////////////////////////////////////////////////////////////

void computeLayoutConstraints(const LayerGroup& group);
void computeLayoutRegion(const LayerGroup& group, const MinMax<Eigen::Array2i>& region);

std::ostream& operator<<(std::ostream& s, const LayerGroup& layout);

}
