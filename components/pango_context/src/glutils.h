#pragma once

#include <pangolin/maths/min_max.h>

namespace pangolin
{

inline void setGlViewport(const MinMax<Eigen::Vector2i>& bounds)
{
    const auto pos = bounds.min();
    const auto size = bounds.range();
    glViewport( pos.x(), pos.y(), size.x()+1, size.y()+1 );
}

inline void setGlScissor(const MinMax<Eigen::Vector2i>& bounds)
{
    const auto pos = bounds.min();
    const auto size = bounds.range();
    glScissor( pos.x(), pos.y(), size.x()+1, size.y()+1 );
}

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
