#ifndef PANGOLIN_PLATFORM_H
#define PANGOLIN_PLATFORM_H

#include <pangolin/config.h>

#ifdef _MSVC_
#define __thread __declspec(thread)
#endif //_MSVC_

#endif // PANGOLIN_PLATFORM_H
