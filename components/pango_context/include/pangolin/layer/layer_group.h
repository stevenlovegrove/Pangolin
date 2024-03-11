#pragma once

#include <Eigen/Core>
#include <pangolin/layer/layer.h>
#include <pangolin/utils/logging.h>

#include <algorithm>
#include <vector>

namespace pangolin
{

////////////////////////////////////////////////////////////////////
/// Represents a (possibly nested) arrangement of Panels on screen
///
class LayerGroup
{
  public:
  enum class Grouping {
    stacked,     // layers blended over one another
    tabbed,      // one layer shown at a time, with user selecting current
    horizontal,  // layers share client area horizontally
    vertical,    // layers share client area vertically
    flex         // layers are arranged in a dynamic group which fills the
                 // available space. Requires common aspect for each layer.
  };

  struct Params {
    Grouping grouping = Grouping::horizontal;
    std::shared_ptr<Layer> layer = nullptr;
    size_t selected_tab = 0;
  };

  struct Constraints {
    // todo: Describe semantic: What is the precise definition of min_pix?  What
    // is the precise definition of parts?
    Eigen::Array2i min_pix = {0, 0};
    Eigen::Array2d parts = {0.0f, 0.0f};
    // Hint / recommendation for what aspect ratio to use.
    AspectRatio aspect = {};
  };

  LayerGroup() = default;
  LayerGroup(const Params& params) : params_(params) {}

  const Params& params() const { return params_; }

  std::optional<Constraints>& constraints() { return constraints_; }
  const std::optional<Constraints>& constraints() const { return constraints_; }

  const std::vector<LayerGroup>& children() const { return children_; }
  std::vector<LayerGroup>& children() { return children_; }

  std::optional<sophus2::Region2I>& region() { return region_; }
  const std::optional<sophus2::Region2I>& region() const { return region_; }

  // Conveniance accessors for children()
  size_t size() const { return children_.size(); }
  LayerGroup& operator[](size_t i) { return children_[i]; }
  const LayerGroup& operator[](size_t i) const { return children_[i]; }

  private:
  std::vector<LayerGroup> children_ = {};

  Params params_ = Params{};
  std::optional<Constraints> constraints_;
  std::optional<sophus2::Region2I> region_;
};

}  // namespace pangolin

#include <pangolin/layer/layer_group_operators.h>
