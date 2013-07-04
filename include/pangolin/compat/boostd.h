#ifndef PANGOLIN_COMPAT_BOOSTD_H
#define PANGOLIN_COMPAT_BOOSTD_H

#ifdef CPP11_NO_BOOST
namespace boostd = std;
#else
namespace boostd = boost;
#endif

#endif // PANGOLIN_COMPAT_MEMORY_H
