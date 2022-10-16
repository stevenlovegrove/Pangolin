#pragma once

#include "common.h"
#include "window.h"

namespace pangolin
{


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
    enum class Profile {
        gl_3_2_core,
        // vulkan ...
    };

    struct Params {
        std::string title = "Pangolin App";
        Window::Size window_size = {1024, 768};
        Profile profile = Profile::gl_3_2_core;
    };

    virtual Shared<Window> window() = 0;

    virtual bool loop(std::function<bool(void)> loop_function) = 0;

    static ExpectShared<Context> Create(Params p);
};

}