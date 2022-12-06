#pragma once

#include <pangolin/utils/shared.h>
#include <string>

namespace pangolin
{

////////////////////////////////////////////////////////////////////
/// The engine represents any global Pangolin state singleton.
/// The footprint of this should be minimal
///
/// Its main short-term purpose is working out the default windowing and graphics
/// parameters, and allowing it to be overridden once and used elsewhere.
///
struct Engine
{
    static Shared<Engine> singleton();

    enum class Profile {
        gl_3_2_core,
        // vulkan ...
    };

    struct{
        std::string window_engine = "default";
        Profile profile = Profile::gl_3_2_core;
    } defaults;
};

}
