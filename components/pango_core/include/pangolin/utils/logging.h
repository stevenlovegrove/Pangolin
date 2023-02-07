#pragma once

#include <farm_ng/core/logging/logger.h>
#include <fmt/format.h>
#include <pangolin/utils/fmt.h>

#ifdef __GNUC__
#define PANGO_FUNCTION __PRETTY_FUNCTION__
#elif (_MSC_VER >= 1310)
#define PANGO_FUNCTION __FUNCTION__
#else
#define PANGO_FUNCTION ""
#endif

// Inspired heavily by farm_ng core logging
// macros: https://github.com/PANGO-ng/PANGO-ng-core

#define PANGO_UNIMPLEMENTED(...)                                               \
  pangolin::Log::instance().log(                                               \
      pangolin::Log::Kind::Unimplemented, __FILE__, PANGO_FUNCTION, __LINE__,  \
      "", ##__VA_ARGS__);

#define PANGO_DEBUG(...)                                                       \
  pangolin::Log::instance().log(                                               \
      pangolin::Log::Kind::Debug, __FILE__, PANGO_FUNCTION, __LINE__, "",      \
      ##__VA_ARGS__);

#define PANGO_INFO(...)                                                        \
  pangolin::Log::instance().log(                                               \
      pangolin::Log::Kind::Info, __FILE__, PANGO_FUNCTION, __LINE__, "",       \
      ##__VA_ARGS__);

#define PANGO_WARN(...)                                                        \
  pangolin::Log::instance().log(                                               \
      pangolin::Log::Kind::Warn, __FILE__, PANGO_FUNCTION, __LINE__, "",       \
      ##__VA_ARGS__);

#define PANGO_ERROR(...)                                                       \
  pangolin::Log::instance().log(                                               \
      pangolin::Log::Kind::Error, __FILE__, PANGO_FUNCTION, __LINE__, "",      \
      ##__VA_ARGS__);

/// Print formatted error message and then panic.
#define PANGO_FATAL(...)                                                       \
  do {                                                                         \
    pangolin::Log::instance().log(                                             \
        pangolin::Log::Kind::Fatal, __FILE__, PANGO_FUNCTION, __LINE__, "",    \
        ##__VA_ARGS__);                                                        \
    ::std::abort();                                                            \
  } while (false)

// Always check. Report but continue.
#define PANGO_CHECK(expr, ...)                                                 \
  if (!(expr)) {                                                               \
    pangolin::Log::instance().log(                                             \
        pangolin::Log::Kind::Error, __FILE__, PANGO_FUNCTION, __LINE__, #expr, \
        ##__VA_ARGS__);                                                        \
  }

// Always check, even in debug. Abort if not true.
#define PANGO_ENSURE(expr, ...)                                                \
  if (!(expr)) {                                                               \
    pangolin::Log::instance().log(                                             \
        pangolin::Log::Kind::Fatal, __FILE__, PANGO_FUNCTION, __LINE__, #expr, \
        ##__VA_ARGS__);                                                        \
    ::std::abort();                                                            \
  }

// May be disabled for optimisation. Abort if not true
#define PANGO_ASSERT(expr, ...)                                                \
  if (!(expr)) {                                                               \
    pangolin::Log::instance().log(                                             \
        pangolin::Log::Kind::Fatal, __FILE__, PANGO_FUNCTION, __LINE__, #expr, \
        ##__VA_ARGS__);                                                        \
    ::std::abort();                                                            \
  }

// May be disabled for optimisation. Abort if not true
#define PANGO_THROW(...)                                                       \
  do {                                                                         \
    pangolin::Log::instance().logAndThrow(                                     \
        pangolin::Log::Kind::Debug, __FILE__, PANGO_FUNCTION, __LINE__, "",    \
        ##__VA_ARGS__);                                                        \
    ::std::abort();                                                            \
  } while (false)

// May be disabled for optimisation. Abort if not true
#define PANGO_THROW_IF(expr, ...)                                              \
  if (expr) {                                                                  \
    pangolin::Log::instance().logAndThrow(                                     \
        pangolin::Log::Kind::Debug, __FILE__, PANGO_FUNCTION, __LINE__, #expr, \
        ##__VA_ARGS__);                                                        \
    ::std::abort();                                                            \
  }

/// Print formatted error message and then panic.
#define PANGO_UNREACHABLE()                                                    \
  do {                                                                         \
    pangolin::Log::instance().log(                                             \
        pangolin::Log::Kind::Fatal, __FILE__, PANGO_FUNCTION, __LINE__, "",    \
        "We've reached code we thought was unreachable.");                     \
    ::std::abort();                                                            \
  } while (false)

#define PANGO_UNWRAP(expr, ...) FARM_UNWRAP(expr, ##__VA_ARGS__)

namespace pangolin
{

struct Log {
  enum class Kind { Debug = 0, Info, Unimplemented, Warn, Error, Fatal };

  static Log& instance();

  virtual void setVerbosity(Kind severity_to_print, bool unique_only) = 0;

  virtual void logImpl(
      Kind level, const char* sFile, const char* sFunction, const int nLine,
      const char* assertion_statement, const std::string& description) = 0;

  template <typename... Args>
  void log(
      Kind kind, const char* sFile, const char* sFunction, const int nLine,
      const char* assertion_statement, Args... args)
  {
    if constexpr (sizeof...(args) > 0) {
      const std::string arg_string = fmt::format(std::forward<Args>(args)...);
      logImpl(kind, sFile, sFunction, nLine, assertion_statement, arg_string);
    } else {
      logImpl(kind, sFile, sFunction, nLine, assertion_statement, "");
    }
  }

  template <typename... Args>
  void logAndThrow(
      Kind kind, const char* sFile, const char* sFunction, const int nLine,
      const char* assertion_statement, Args... args)
  {
    if constexpr (sizeof...(args) > 0) {
      const std::string arg_string = fmt::format(std::forward<Args>(args)...);
      logImpl(kind, sFile, sFunction, nLine, assertion_statement, arg_string);
      throw std::runtime_error(arg_string);
    } else {
      logImpl(kind, sFile, sFunction, nLine, assertion_statement, "");
      throw std::runtime_error(assertion_statement);
    }
  }
};

std::ostream& operator<<(std::ostream& o, Log::Kind kind);

}  // namespace pangolin
