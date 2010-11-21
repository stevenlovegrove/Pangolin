#ifndef PANGOLIN_PLATFORM_H
#define PANGOLIN_PLATFORM_H

#include <pangolin/config.h>

#ifdef MSVC
#define __thread __declspec(thread)
#endif //MSVC

#endif // PANGOLIN_PLATFORM_H
