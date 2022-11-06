#pragma once

#include <pangolin/maths/min_max.h>
#include <pangolin/gl/gl.h>

namespace pangolin
{

// Will glEnable and glDisable GL capability regardless
// of current state.
struct [[nodiscard]] ScopedGlEnable
{
    ScopedGlEnable(GLenum cap) : cap_(cap) { glEnable(cap_); }
    ~ScopedGlEnable() { glDisable(cap_); }
    GLenum cap_;
};

// Will glDisable and glEnable GL capability regardless
// of current state.
struct [[nodiscard]] ScopedGlDisable
{
    ScopedGlDisable(GLenum cap) : cap_(cap) { glDisable(cap_); }
    ~ScopedGlDisable() { glEnable(cap_); }
    GLenum cap_;
};


}
