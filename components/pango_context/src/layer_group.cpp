#include <pangolin/gui/layer_group.h>

namespace pangolin
{

struct FlexArrangement {
  static double constexpr row_height = 1.0;
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
    double const flex_aspect = flexAspect();
    double const aspect_efficiency = target_aspect >= flex_aspect
                                         ? flex_aspect / target_aspect
                                         : target_aspect / flex_aspect;
    double const fill_efficiency = elementArea() / flexArea();
    return aspect_efficiency * fill_efficiency;
  }
};

std::optional<FlexArrangement> getFlexRows(
    std::vector<double> const& el_width, double row_width)
{
  FlexArrangement flex = {
      .row_width = row_width,
      .num_rows = 1,
  };

  double row_total = 0.0;

  for (size_t i = 0; i < el_width.size(); ++i) {
    double const w = el_width[i];
    flex.width_sum += w;

    if (w > row_width) return getFlexRows(el_width, w);

    double const width_with_el = row_total + w;
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

void computeLayoutConstraints(LayerGroup const& group)
{
  LayerGroup::LayoutInfo total = {};

  double aspect_sum = 0.0;
  int aspect_n = 0;

  // 1st pass, bottom up
  for (auto const& child : group.children) {
    // ask child to compute its layout size and cache
    computeLayoutConstraints(child);
    auto& x_info = child.cached_;

    // Update our constraints base on those of the children
    switch (group.grouping) {
      case LayerGroup::Grouping::stacked:
        [[fallthrough]];
      case LayerGroup::Grouping::tabbed:
        total.min_pix = max(total.min_pix, x_info.min_pix);
        total.parts = max(total.parts, x_info.parts);
        if (x_info.aspect_hint) {
          aspect_sum += x_info.aspect_hint;
          ++aspect_n;
        }
        break;
      case LayerGroup::Grouping::horizontal:
        total.min_pix[0] += x_info.min_pix[0];
        total.parts[0] += x_info.parts[0];
        total.min_pix[1] = max(total.min_pix[1], x_info.min_pix[1]);
        total.parts[1] = max(total.parts[1], x_info.parts[1]);
        if (x_info.aspect_hint) {
          aspect_sum += x_info.aspect_hint;
          ++aspect_n;
        }
        break;
      case LayerGroup::Grouping::vertical:
        total.min_pix[1] += x_info.min_pix[1];
        total.parts[1] += x_info.parts[1];
        total.min_pix[0] = max(total.min_pix[0], x_info.min_pix[0]);
        total.parts[0] = max(total.parts[0], x_info.parts[0]);
        if (x_info.aspect_hint) {
          aspect_sum += 1.0 / x_info.aspect_hint;
          ++aspect_n;
        }
        break;
      case LayerGroup::Grouping::flex:
        // flex layout is pretty heuristic-ey
        double part_area = x_info.parts[0] * x_info.parts[1];
        double part_w = x_info.aspect_hint * part_area;
        double part_h = part_area / part_w;
        total.parts[0] += part_w;
        total.parts[1] += part_h;
        if (x_info.aspect_hint) {
          aspect_sum += x_info.aspect_hint;
          ++aspect_n;
        }
        break;
    }
  }

  switch (group.grouping) {
    case LayerGroup::Grouping::stacked:
    case LayerGroup::Grouping::tabbed:
    case LayerGroup::Grouping::flex:
      // Use average aspect
      total.aspect_hint = aspect_n ? aspect_sum / aspect_n : 0.0;
      break;
    case LayerGroup::Grouping::horizontal:
      // Use sum
      total.aspect_hint = aspect_sum;
      break;
    case LayerGroup::Grouping::vertical:
      // Use sum of inverse, inversed.
      total.aspect_hint = aspect_sum ? 1.0 / aspect_sum : 0.0;
      break;
  }

  // Finally, apply any constraints from the group layer, if it exists
  if (group.layer) {
    double const aspect_hint = group.layer->aspectHint();
    if (aspect_hint) total.aspect_hint = aspect_hint;

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
          group.layer->sizeHint()[i]);
    }
  }

  // cache for later 2nd pass
  group.cached_ = total;
}

void computeLayoutRegion(
    LayerGroup const& group, MinMax<Eigen::Array2i> const& region)
{
  // 2nd pass, top down

  auto& r = group.cached_;
  r.region = region;

  int constexpr handle_pix = 5;
  int const num_children = group.children.size();
  int const total_handle_pix = handle_pix * (num_children - 1);

  switch (group.grouping) {
    case LayerGroup::Grouping::stacked:
      [[fallthrough]];
    case LayerGroup::Grouping::tabbed:
      // the same region for stacked and tabbed.
      for (auto const& child : group.children) {
        computeLayoutRegion(child, region);
      }
      break;
    case LayerGroup::Grouping::horizontal: {
      // compute how many pixels we have to spend after accounting fixed stuff
      int remaining = region.range()[0] - r.min_pix[0] - total_handle_pix;

      // Compute the size of one ratio unit given total and pixels we have
      // to fill.
      double const ratio_unit_pix = remaining / r.parts[0];

      int x = region.min()[0];
      for (auto const& child : group.children) {
        int const w = int(
            child.cached_.min_pix[0] + child.cached_.parts[0] * ratio_unit_pix);
        MinMax<Eigen::Array2i> const child_region(
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
      int remaining = region.range()[1] - r.min_pix[1] - total_handle_pix;

      // Compute the size of one ratio unit given total and pixels we have
      // to fill.
      double const ratio_unit_pix = remaining / r.parts[1];

      int y = region.min()[1];
      for (auto const& child : group.children) {
        int const h = int(
            child.cached_.min_pix[1] + child.cached_.parts[1] * ratio_unit_pix);
        MinMax<Eigen::Array2i> const child_region(
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

      auto const region_size = region.range();
      double const target_aspect = double(region_size[0]) / region_size[1];

      // children widths if they were to maintain aspect and have same
      // unit height
      std::vector<double> ws;
      double total_width = 0.0;

      for (auto const& child : group.children) {
        double const aspect =
            child.cached_.aspect_hint ? child.cached_.aspect_hint : 1.0;
        double const w = aspect;
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
          double const eff = maybe_flex->efficiency(target_aspect);
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

      double const flex_aspect = best->flexAspect();
      double const unit_size_pix = flex_aspect > target_aspect
                                       ? region.range().x() / best->row_width
                                       : region.range().y() / best->num_rows;
      int const row_height_pix = int(unit_size_pix);

      int x = region.min()[0];
      int y = region.min()[1];
      for (auto const& child : group.children) {
        double const aspect =
            child.cached_.aspect_hint ? child.cached_.aspect_hint : 1.0;
        int const h = row_height_pix;
        int const w = int(unit_size_pix * aspect);
        if ((x + w) > region.max()[0]) {
          x = region.min()[0];
          y += h;
        }
        MinMax<Eigen::Array2i> const child_region(
            Eigen::Array2i(x, y), Eigen::Array2i(x + w - 1, y + h - 1));
        computeLayoutRegion(child, child_region);
        x += w;
      }
    }
  }
}

std::ostream& operator<<(std::ostream& s, LayerGroup const& layout)
{
  auto const& v = layout.children;
  FARM_CHECK(v.size() > 0);
  if (layout.layer) s << "x";
  s << "(";
  s << v[0];
  for (size_t i = 1; i < v.size(); ++i) {
    switch (layout.grouping) {
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
