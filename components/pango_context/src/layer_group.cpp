#include <pangolin/layer/layer_group.h>

using namespace sophus;

namespace pangolin
{

struct FlexArrangement {
  constexpr static double row_height = 1.0;
  double row_width = 0.0;
  double width_sum = 0.0;
  size_t num_rows = 0.0;
  double lowest_col_margin = std::numeric_limits<double>::max();

  double elementArea() const { return width_sum * row_height; }

  double flexArea() const { return row_width * num_rows * row_height; }

  double flexAspect() const
  {
    return row_width / double(num_rows * row_height);
  }

  double efficiency(double target_aspect) const
  {
    const double flex_aspect = flexAspect();
    const double aspect_efficiency = target_aspect >= flex_aspect
                                         ? flex_aspect / target_aspect
                                         : target_aspect / flex_aspect;
    const double fill_efficiency = elementArea() / flexArea();
    return aspect_efficiency * fill_efficiency;
  }
};

std::optional<FlexArrangement> getFlexRows(
    const std::vector<double>& el_width, double row_width)
{
  FlexArrangement flex = {
      .row_width = row_width,
      .num_rows = 1,
  };

  double row_total = 0.0;

  for (size_t i = 0; i < el_width.size(); ++i) {
    const double w = el_width[i];
    flex.width_sum += w;

    if (w > row_width) return getFlexRows(el_width, w);

    const double width_with_el = row_total + w;
    if (width_with_el <= row_width) {
      // add to row
      row_total = width_with_el;
    } else {
      // start new row
      flex.lowest_col_margin =
          std::min(flex.lowest_col_margin, width_with_el - row_width);
      ++flex.num_rows;
      row_total = w;
    }
  }
  return flex;
}

void computeLayoutConstraints(LayerGroup& group)
{
  LayerGroup::Constraints total = {};

  double aspect_sum = 0.0;
  int aspect_n = 0;

  const LayerGroup::Params params = group.params();

  // 1st pass, bottom up
  for (LayerGroup& child : group.children()) {
    // Recursive call: ask child to compute its layout constraints
    computeLayoutConstraints(child);
    const LayerGroup::Constraints& child_constraint =
        PANGO_UNWRAP(child.constraints());

    // Update our constraints base on those of the children
    switch (params.grouping) {
      case LayerGroup::Grouping::stacked:
        [[fallthrough]];
      case LayerGroup::Grouping::tabbed:
        // the same constraints for stacked and tabbed.
        total.min_pix = max(total.min_pix, child_constraint.min_pix);
        total.parts = max(total.parts, child_constraint.parts);
        if (child_constraint.aspect.hasHint()) {
          aspect_sum += child_constraint.aspect.hint();
          ++aspect_n;
        }
        break;
      case LayerGroup::Grouping::horizontal:
        total.min_pix[0] += child_constraint.min_pix[0];
        total.parts[0] += child_constraint.parts[0];
        total.min_pix[1] = max(total.min_pix[1], child_constraint.min_pix[1]);
        total.parts[1] = max(total.parts[1], child_constraint.parts[1]);
        if (child_constraint.aspect.hasHint()) {
          aspect_sum += child_constraint.aspect.hint();
          ++aspect_n;
        }
        break;
      case LayerGroup::Grouping::vertical:
        total.min_pix[1] += child_constraint.min_pix[1];
        total.parts[1] += child_constraint.parts[1];
        total.min_pix[0] = max(total.min_pix[0], child_constraint.min_pix[0]);
        total.parts[0] = max(total.parts[0], child_constraint.parts[0]);
        if (child_constraint.aspect.hasHint()) {
          aspect_sum += 1.0 / child_constraint.aspect.hint();
          ++aspect_n;
        }
        break;
      case LayerGroup::Grouping::flex:
        // flex layout is pretty heuristic-ey
        double part_area =
            child_constraint.parts[0] * child_constraint.parts[1];
        double part_w = child_constraint.aspect.hint() * part_area;
        double part_h = part_area / part_w;
        total.parts[0] += part_w;
        total.parts[1] += part_h;
        if (child_constraint.aspect.hasHint()) {
          aspect_sum += child_constraint.aspect.hint();
          ++aspect_n;
        }
        break;
    }
  }

  switch (params.grouping) {
    case LayerGroup::Grouping::stacked:
    case LayerGroup::Grouping::tabbed:
    case LayerGroup::Grouping::flex:
      // Use average aspect
      total.aspect.setHint(aspect_n ? aspect_sum / aspect_n : 0.0);
      break;
    case LayerGroup::Grouping::horizontal:
      // Use sum, fall back to 1.0 if aspect_sum is 0.0, which most likely means
      // that aspect_n==0.
      total.aspect.setHint(aspect_sum == 0.0 ? 1.0 : aspect_sum);
      break;
    case LayerGroup::Grouping::vertical:
      // Use sum of inverse, inversed.
      total.aspect.setHint(aspect_sum == 0.0 ? 1.0 / aspect_sum : 0.0);
      break;
  }

  // Finally, apply any constraints from the group layer, if it exists
  if (params.layer) {
    const auto layer_aspect = params.layer->aspectHint();
    if (layer_aspect.hasHint()) {
      // If the aspect hint from layer if there is one. In this case, aspect
      // ratio hints from children are ignored.
      total.aspect = layer_aspect;
    }

    for (int i = 0; i < 2; ++i) {
      std::visit(
          overload{
              [&](Parts part) {
                total.parts[i] = max(total.parts[i], part.ratio);
              },
              [&](Pixels ab) {
                total.min_pix[i] = max(total.min_pix[i], ab.pixels);
              },
          },
          params.layer->sizeHint()[i]);
    }
  }

  // set constraints for later 2nd pass
  group.constraints() = total;
}

void computeLayoutRegion(LayerGroup& group, const Region2I& region)
{
  // 2nd pass, top down
  group.region() = region;

  constexpr int handle_pix = 5;
  const int num_children = group.children().size();
  const int total_handle_pix = handle_pix * (num_children - 1);
  const auto& constraints = PANGO_UNWRAP(group.constraints());

  if (num_children == 0) {
    // nothing to do.
    return;
  }

  switch (group.params().grouping) {
    case LayerGroup::Grouping::stacked:
      [[fallthrough]];
    case LayerGroup::Grouping::tabbed:
      // the same region for stacked and tabbed.
      for (auto& child : group.children()) {
        computeLayoutRegion(child, region);
      }
      break;
    case LayerGroup::Grouping::horizontal: {
      // compute how many pixels we have to spend after accounting fixed stuff
      int remaining =
          region.range()[0] - constraints.min_pix[0] - total_handle_pix;

      // Compute the size of one ratio unit given total and pixels we have
      // to fill.
      const double ratio_unit_pix = remaining / constraints.parts[0];

      int x = region.min()[0];
      for (auto& child : group.children()) {
        const auto& child_constraints = PANGO_UNWRAP(child.constraints());

        const int w =
            int(child_constraints.min_pix[0] +
                child_constraints.parts[0] * ratio_unit_pix);
        const Region2I child_region = Region2I::fromMinMax(
            Eigen::Array2i(x, region.min()[1]),
            Eigen::Array2i(x + w - 1, region.max()[1]));
        computeLayoutRegion(child, child_region);
        x += w + handle_pix;
      }
      // TODO: we'll have a couple of pixels left from rounding down
      break;
    }
    case LayerGroup::Grouping::vertical: {
      // compute how many pixels we have to spend after accounting fixed stuff
      int remaining =
          region.range()[1] - constraints.min_pix[1] - total_handle_pix;

      // Compute the size of one ratio unit given total and pixels we have
      // to fill.
      const double ratio_unit_pix = remaining / constraints.parts[1];

      int y = region.min()[1];
      for (auto& child : group.children()) {
        const auto& child_constraints = PANGO_UNWRAP(child.constraints());

        const int h =
            int(child_constraints.min_pix[1] +
                child_constraints.parts[1] * ratio_unit_pix);
        const auto child_region = Region2I::fromMinMax(
            Eigen::Array2i(region.min()[0], y),
            Eigen::Array2i(region.max()[0], y + h - 1));
        computeLayoutRegion(child, child_region);
        y += h + handle_pix;
      }
      // TODO: we'll have a couple of pixels left from rounding down
      break;
    }
    case LayerGroup::Grouping::flex: {
      // everything here is in normalized units where h=1 and w = aspect

      const auto region_size = region.range();
      const double target_aspect = double(region_size[0]) / region_size[1];

      // children widths if they were to maintain aspect and have same
      // unit height
      std::vector<double> ws;
      double total_width = 0.0;

      for (const auto& child : group.children()) {
        const auto& child_constraints = PANGO_UNWRAP(child.constraints());

        const double aspect = child_constraints.aspect.hasHint()
                                  ? child_constraints.aspect.hint()
                                  : 1.0;
        const double w = aspect;
        ws.push_back(w);
        total_width += w;
      }

      double col_break = sqrt(target_aspect * (total_width * 1.0));
      std::optional<FlexArrangement> best;
      double best_eff = 0.0;

      // try just a few
      for (int i = 0; i < 4; ++i) {
        auto maybe_flex = getFlexRows(ws, col_break);
        if (maybe_flex) {
          const double eff = maybe_flex->efficiency(target_aspect);
          if (eff > best_eff) {
            best_eff = eff;
            best = maybe_flex;
          }

          // increment col_break to next one that will make a differenec
          if (maybe_flex->lowest_col_margin < col_break) {
            col_break += maybe_flex->lowest_col_margin;
          } else {
            break;
          }
        }
      }

      PANGO_ENSURE(best, "Problem packing for flex layout.");

      const double flex_aspect = best->flexAspect();
      const double unit_size_pix = flex_aspect > target_aspect
                                       ? region.range().x() / best->row_width
                                       : region.range().y() / best->num_rows;
      const int row_height_pix = int(unit_size_pix);

      int x = region.min()[0];
      int y = region.min()[1];
      for (auto& child : group.children()) {
        const auto& child_constraints = PANGO_UNWRAP(child.constraints());

        const double aspect = child_constraints.aspect.hasHint()
                                  ? child_constraints.aspect.hint()
                                  : 1.0;
        const int h = row_height_pix;
        const int w = int(unit_size_pix * aspect);
        if ((x + w) > region.max()[0]) {
          x = region.min()[0];
          y += h;
        }
        const auto child_region = Region2I::fromMinMax(
            Eigen::Array2i(x, y), Eigen::Array2i(x + w - 1, y + h - 1));
        computeLayoutRegion(child, child_region);
        x += w;
      }
    }
  }
}

std::ostream& operator<<(std::ostream& s, const LayerGroup& layout)
{
  const auto& v = layout.children();
  if (layout.params().layer) {
    s << "x";
  }
  if (layout.region()) {
    s << "[" << layout.region()->min().transpose() << ", "
      << layout.region()->max().transpose() << "]";
  }

  if (v.empty()) {
    return s;
  }
  s << "(";
  s << v[0];
  for (size_t i = 1; i < v.size(); ++i) {
    switch (layout.params().grouping) {
      case LayerGroup::Grouping::horizontal:
        s << "|";
        break;
      case LayerGroup::Grouping::vertical:
        s << "/";
        break;
      case LayerGroup::Grouping::tabbed:
        s << ",";
        break;
      case LayerGroup::Grouping::stacked:
        s << "^";
        break;
      default:
        break;
    }
    s << v[i];
  }
  s << ")";
  return s;
}

}  // namespace pangolin
