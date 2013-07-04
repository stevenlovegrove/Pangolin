#ifndef PANGOLIN_COMPAT_MEMORY_H
#define PANGOLIN_COMPAT_MEMORY_H

#include <pangolin/platform.h>

#ifdef CPP11_NO_BOOST
    #include <memory>
#else
    #include <boost/shared_ptr.hpp>
#endif

#include <pangolin/compat/boostd.h>

#endif // PANGOLIN_COMPAT_MEMORY_H
