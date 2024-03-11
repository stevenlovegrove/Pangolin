#pragma once

#include <pangolin/layer/layer_group.h>
#include <pangolin/utils/concept_utils.h>
#include <pangolin/utils/variant_overload.h>
#include <sophus2/concepts/point.h>
#include <sophus2/image/dyn_image_types.h>

namespace pangolin
{

////////////////////////////////////////////////////////////////////
// LayerTraits is a utility that can be specialized for different types to
// support direct addition to context layout using the layout operators
template <typename T>
struct LayerConversionTraits;

template <DerivedFrom<Layer> L>
struct LayerConversionTraits<Shared<L>> {
  static Shared<Layer> makeLayer(const Shared<L>& layer) { return layer; }
};

template <DerivedFrom<Layer> L>
struct LayerConversionTraits<std::shared_ptr<L>> {
  static Shared<Layer> makeLayer(const std::shared_ptr<L>& layer)
  {
    return layer;
  }
};

////////////////////////////////////////////////////////////////////

namespace detail
{
template <class T, class U>
concept SameHelper = std::is_same_v<T, U>;
}

template <class T, class U>
concept same_as = detail::SameHelper<T, U> && detail::SameHelper<U, T>;

// Concept to accept types where the LayerTraits specialization has been defined
template <typename T>
concept LayerConvertable = requires(T x)
{
  {
    Shared<Layer>(LayerConversionTraits<T>::makeLayer(x))
    } -> SameAs<Shared<Layer>>;
};

template <typename T>
concept LayerGroupConvertable =
    LayerConvertable<T> || same_as<std::decay_t<T>, LayerGroup>;

template <LayerConvertable T>
auto makeLayer(const T& v)
{
  return LayerConversionTraits<T>::makeLayer(v);
}

template <LayerGroupConvertable T>
LayerGroup makeLayerGroup(const T& v)
{
  using BaseT = std::decay_t<T>;
  if constexpr (same_as<BaseT, LayerGroup>) {
    return v;
  } else {
    return LayerGroup({.layer = Shared<Layer>(makeLayer(v))});
  }
}

namespace detail
{
// Implementation of how layergroups are composed (and simplified)
inline LayerGroup join(
    LayerGroup::Grouping op_type, const LayerGroup& lhs, const LayerGroup& rhs)
{
  PANGO_ASSERT(lhs.children().size() > 0 || lhs.params().layer);
  PANGO_ASSERT(rhs.children().size() > 0 || rhs.params().layer);

  LayerGroup ret({.grouping = op_type});

  if (op_type == lhs.params().grouping && !lhs.params().layer) {
    // We can merge hierarchy
    ret.children() = lhs.children();
  } else {
    ret.children().push_back(lhs);
  }

  if (op_type == rhs.params().grouping && !rhs.params().layer) {
    // We can merge hierarchy
    ret.children().insert(
        ret.children().end(), rhs.children().begin(), rhs.children().end());
  } else {
    ret.children().push_back(rhs);
  }

  return ret;
}
}  // namespace detail

////////////////////////////////////////////////////////////////////
// Define syntactic sugar for composing layergroups to make layouts

#define PANGO_PANEL_OPERATOR(op, op_type)                                      \
  template <LayerGroupConvertable LHS, LayerGroupConvertable RHS>              \
  LayerGroup op(const LHS& lhs, const RHS& rhs)                                \
  {                                                                            \
    return detail::join(op_type, makeLayerGroup(lhs), makeLayerGroup(rhs));    \
  }

#define PANGO_COMMA ,
PANGO_PANEL_OPERATOR(operator PANGO_COMMA, LayerGroup::Grouping::tabbed)
PANGO_PANEL_OPERATOR(operator|, LayerGroup::Grouping::horizontal)
PANGO_PANEL_OPERATOR(operator/, LayerGroup::Grouping::vertical)
PANGO_PANEL_OPERATOR(operator^, LayerGroup::Grouping::stacked)
#undef PANGO_PANEL_OPERATOR
#undef PANGO_COMMA

template <typename T>
LayerGroup flex(T head)
{
  LayerGroup g({.grouping = LayerGroup::Grouping::flex});
  g.children().push_back(makeLayerGroup(head));
  return g;
}

template <typename T, typename... TArgs>
LayerGroup flex(T head, TArgs... args)
{
  return detail::join(
      LayerGroup::Grouping::flex, makeLayerGroup(head),
      flex(std::forward<TArgs>(args)...));
}

////////////////////////////////////////////////////////////////////

// Calculates and populates constraints for given layout group recursively.
//
// Side-effect: The constraints for the give group and all its children are
// calculated and populated recursively in a bottom ups fashion.
void computeLayoutConstraints(LayerGroup& group);

// Populates region in layer group in top down-fashion.
//
// Side-effects:
//  - Provided region is assigned to group.
//  - Recursively, regions are calculated for all children and assign to them.
//
// Precondition: The constraints of the group and its children must be populated
// already (e.g. by corresponding call to populateConstraintsRecursively).
void computeLayoutRegion(LayerGroup& group, const sophus2::Region2I& region);

std::ostream& operator<<(std::ostream& s, const LayerGroup& layout);

}  // namespace pangolin
