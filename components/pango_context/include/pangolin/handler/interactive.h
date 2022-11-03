#pragma once

#include <pangolin/utils/flag_set.h>
#include <pangolin/maths/conventions.h>
#include <pangolin/gui/layer.h>

namespace pangolin
{

struct WindowPosition
{
  const MinMax<Eigen::Array2d>& region() const
  {
    return region_;
  }

  Eigen::Array2d posInWindow() const
  {
    return pos_window_;
  }

  Eigen::Array2d posInRegion() const
  {
    return pos_window_ - region_.min();
  }

  Eigen::Array2d posInRegionNorm() const
  {
    return (pos_window_ - region_.min()) / region_.range();
  }

  MinMax<Eigen::Array2d> region_;
  Eigen::Array2d pos_window_;
};

enum class PointerAction {
    hover, down, drag, drag_up, click_up, double_click_up
};

enum class PointerButton {
    primary, secondary, tertiary,
    back, forward, _
};

enum class ModifierKey {
    win_cmd_meta, shift, ctrl, fn, alt_option, _
};

struct Interactive {
    using PointerButtonStatus = flag_set<PointerButton>;
    using ModifierKeyStatus = flag_set<ModifierKey>;

    struct PointerEvent {
        PointerAction action;
        std::optional<PointerButton> button;
        PointerButtonStatus pointer_pressed;
        ModifierKeyStatus modifier_held;
    };

    struct ScrollEvent {
        Eigen::Vector2d pan;
        Eigen::Vector2d tilt;
        double zoom;
    };

    struct Event {
      WindowPosition pointer_pos;
      std::variant<PointerEvent,ScrollEvent> detail;
    };

    virtual bool handleEvent(const Event&) = 0;
};

}
