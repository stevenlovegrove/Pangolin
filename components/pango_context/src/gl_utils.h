#pragma once

#include <pangolin/maths/min_max.h>
#include <pangolin/gl/gl.h>

namespace pangolin
{

// Will glEnable and glDisable GL capability regardless
// of current state.
struct [[nodiscard]] ScopedGlEnable
{
    static ScopedGlEnable noOp() {return ScopedGlEnable(0);}
    ScopedGlEnable(GLenum cap) : cap_(cap) { if(cap_) glEnable(cap_); }
    ~ScopedGlEnable() { if(cap_) glDisable(cap_); }
    GLenum cap_;
};

// Will glDisable and glEnable GL capability regardless
// of current state.
struct [[nodiscard]] ScopedGlDisable
{
    static ScopedGlDisable noOp() {return ScopedGlDisable(0);}
    ScopedGlDisable(GLenum cap) : cap_(cap) { if(cap_) glDisable(cap_); }
    ~ScopedGlDisable() { if(cap_) glEnable(cap_); }
    GLenum cap_;
};


}
