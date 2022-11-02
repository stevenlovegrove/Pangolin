#pragma once

#include <pangolin/utils/fmt.h>

// Inspired heavily by farm_ng core logging
// macros: https://github.com/PANGO-ng/PANGO-ng-core

#define PANGO_UNIMPLEMENTED(...) \
  Log::instance().log(Log::Kind::Unimplemented, __FILE__, __LINE__, ##__VA_ARGS__);

#define PANGO_DEBUG(...) \
  Log::instance().log(Log::Kind::Debug, __FILE__, __LINE__, ##__VA_ARGS__);

#define PANGO_INFO(...) \
  Log::instance().log(Log::Kind::Info, __FILE__, __LINE__, ##__VA_ARGS__);

#define PANGO_WARN(...) \
  Log::instance().log(Log::Kind::Warn, __FILE__, __LINE__, ##__VA_ARGS__);

#define PANGO_ERROR(...) \
  Log::instance().log(Log::Kind::Error, __FILE__, __LINE__, ##__VA_ARGS__);

/// Print formatted error message and then panic.
#define PANGO_FATAL(...) \
  do { \
    Log::instance().log(Log::Kind::Fatal, __FILE__, __LINE__, ##__VA_ARGS__); \
    ::std::abort(); \
  }while(false)

namespace pangolin
{

struct Log
{
    enum class Kind {
        Debug=0, Info, Unimplemented, Warn, Error, Fatal
    };

    static Log& instance();

    virtual void setVerbosity(Kind severity_to_print, bool unique_only) = 0;

    virtual void logImpl(Kind level, const char *sFile, const int nLine, const std::string& description) = 0;

    template<typename... Args>
    void log(Kind kind, const char *sFile, const int nLine, Args... args)
    {
        const std::string arg_string = fmt::format(std::forward<Args>(args)...);
        logImpl(kind, sFile, nLine, arg_string);
    }
};

std::ostream& operator<<(std::ostream& o, Log::Kind kind);

}
