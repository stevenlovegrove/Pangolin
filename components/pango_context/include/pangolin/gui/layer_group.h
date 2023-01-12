#pragma once

#include <Eigen/Core>
#include <pangolin/gui/layer.h>
#include <pangolin/utils/logging.h>

#include <algorithm>
#include <vector>

namespace pangolin
{

////////////////////////////////////////////////////////////////////
/// Represents a (possibly nested) arrangement of Panels on screen
///
struct LayerGroup {
  enum class Grouping {
    stacked,     // layers blended over one another
    tabbed,      // one layer shown at a time, with user selecting current
    horizontal,  // layers share client area horizontally
    vertical,    // layers share client area vertically
    flex         // layers are arranged in a dynamic group which fills the
                 // available space. Requires common aspect for each layer.
  };

  LayerGroup() = default;
  LayerGroup(Shared<Layer> layer) : layer(layer) {}

  Grouping grouping = Grouping::horizontal;
  std::vector<LayerGroup> children = {};
  std::shared_ptr<Layer> layer = nullptr;
  size_t selected_tab = 0;

  struct LayoutInfo {
    Eigen::Array2i min_pix = {0, 0};
    Eigen::Array2d parts = {0.0f, 0.0f};
    double aspect_hint = 0.0;
    Region2I region = Region2I::empty();
  };

  mutable LayoutInfo cached_;
};

}  // namespace pangolin

#include <pangolin/gui/layer_group_operators.h>
