#pragma once

#include <array>
#include <pangolin/utils/shared.h>
#include <pangolin/context/engine.h>

namespace pangolin
{

struct WindowInterface;
struct PanelGroup;

////////////////////////////////////////////////////////////////////
/// Represents the pangolin context for one Window or Window-like
/// offscreen buffer. A 'graphics' context is likely one-to-one with
/// this object, but that is an implementation detail and may not
/// always be true. For example a particular implementation may
/// internally share one graphics context between multiple Pangolin
/// Contexts
///
struct Context : std::enable_shared_from_this<Context>
{
    using Window = WindowInterface;

    struct Params {
        std::string title = "Pangolin App";
        std::array<int,2> window_size = {1024, 768};
        std::string window_engine = Engine::singleton()->defaults.window_engine;
        Engine::Profile profile = Engine::singleton()->defaults.profile;
    };

    virtual Shared<Window> window() = 0;

    virtual void loop(std::function<bool(void)> loop_function) = 0;

    virtual void setLayout(const Shared<PanelGroup>& layout) = 0;

    static Shared<Context> Create(Params p);
};

}
