#pragma once

#include <Eigen/Core>
#include <pangolin/layer/interactive.h>
#include <pangolin/maths/region.h>
#include <pangolin/utils/shared.h>

#include <variant>

namespace pangolin
{

struct Context;

// Layer length unit: pixels
struct Pixels {
  int pixels = 100;
};

// Layer length unit: ratio
struct Parts {
  double ratio = 1.0;
};

// Unit of layer length (e.g. width, height), either in pixels or in "parts".
using LayerLengthUnit = std::variant<Parts, Pixels>;

// Layer size (width and height).
using LayerSize = Eigen::Vector<LayerLengthUnit, 2>;

struct AspectRatio {
  AspectRatio() = default;

  static AspectRatio one()
  {
    AspectRatio aspect;
    aspect.setHint(1.0);
    return aspect;
  }

  static AspectRatio fromRatio(double ratio)
  {
    AspectRatio aspect;
    aspect.setHint(ratio);
    return aspect;
  }

  bool hasHint() const { return width_by_height_hint_ != 0.0; }

  double hint() const
  {
    PANGO_ASSERT(this->hasHint());
    return width_by_height_hint_;
  }

  // Precondition: ratio hint must be > 0.0.
  void setHint(double ratio_hint)
  {
    PANGO_ASSERT(ratio_hint >= 0.0, "got: {}", ratio_hint);
    width_by_height_hint_ = ratio_hint;
  }

  private:
  // precondition: width_by_height_hint_ >= 0.0
  double width_by_height_hint_ = 0.0;  // 0.0 means we have no hint
};

////////////////////////////////////////////////////////////////////
/// Represents a client area in a window with layout handling
///
struct Layer : public Interactive {
  virtual ~Layer() {}

  using Size = LayerSize;

  struct RenderParams {
    Region2I region;
  };

  virtual std::string name() const = 0;

  virtual LayerSize sizeHint() const = 0;

  virtual AspectRatio aspectHint() const { return AspectRatio{}; };

  virtual void renderIntoRegion(const Context&, const RenderParams&) = 0;

  // By default, we ignore all input events
  virtual bool handleEvent(const Context&, const Event&) override
  {
    return false;
  }

  struct Params {
    std::string name = "";
    LayerSize size_hint = {Parts{1}, Parts{1}};
  };
  static Shared<Layer> Create(Params);

  // Special convenience layers for clearing color-buffer, depth-buffer, or
  // both
  static Shared<Layer> ClearColor();
  static Shared<Layer> ClearZ();
  static Shared<Layer> Clear();
};

}  // namespace pangolin
