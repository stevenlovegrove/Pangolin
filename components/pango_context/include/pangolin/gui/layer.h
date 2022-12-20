#pragma once

#include <Eigen/Core>
#include <pangolin/gui/interactive.h>
#include <pangolin/maths/min_max.h>
#include <pangolin/utils/shared.h>

#include <variant>

namespace pangolin
{

struct Context;

struct Pixels {
  int pixels = 100;
};
struct Parts {
  double ratio = 1.0;
};

////////////////////////////////////////////////////////////////////
/// Represents a client area in a window with layout handling
///
struct Layer : public Interactive {
  virtual ~Layer() {}

  using Dim = std::variant<Parts, Pixels>;
  using Size = Eigen::Vector<Dim, 2>;

  struct RenderParams {
    MinMax<Eigen::Array2i> region;
  };

  virtual std::string name() const = 0;

  virtual Size sizeHint() const = 0;

  virtual double aspectHint() const { return 0; };

  virtual void renderIntoRegion(const Context&, const RenderParams&) = 0;

  // By default, we ignore all input events
  virtual bool handleEvent(const Context&, const Event&) override
  {
    return false;
  }

  struct Params {
    std::string name = "";
    Size size_hint = {Parts{1}, Parts{1}};
  };
  static Shared<Layer> Create(Params);

  // Special convenience layers for clearing color-buffer, depth-buffer, or
  // both
  static Shared<Layer> ClearColor();
  static Shared<Layer> ClearZ();
  static Shared<Layer> Clear();
};

}  // namespace pangolin
