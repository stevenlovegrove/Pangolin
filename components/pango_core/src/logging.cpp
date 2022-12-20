#include <fmt/color.h>
#include <fmt/ostream.h>
#include <pangolin/utils/logging.h>

#include <chrono>
#include <map>
#include <mutex>

namespace pangolin
{
using LogKey = std::pair<const char*, int>;
}
namespace std
{
template <>
struct hash<pangolin::LogKey> {
  size_t operator()(const pangolin::LogKey& x) const
  {
    return hash<const char*>()(x.first) + 13441 * hash<int>()(x.second);
  }
};
}  // namespace std

namespace pangolin
{

constexpr size_t kNumLogKinds = 6;
constexpr const char* sLogKindNameTable[] = {
    "DEBUG:", "INFO: ", "UNIMP:", "WARN: ", "ERROR:", "FATAL:"};

constexpr fmt::color sLogKindColorTable[] = {
    fmt::color::medium_violet_red, fmt::color::deep_sky_blue,
    fmt::color::light_yellow,      fmt::color::orange_red,
    fmt::color::crimson,           fmt::color::rebecca_purple};

std::ostream& operator<<(std::ostream& o, Log::Kind kind)
{
  o << sLogKindNameTable[static_cast<int>(kind)];
  return o;
}

struct LoggingImpl : public Log {
  ~LoggingImpl()
  {
    if (print_summary_on_close_ && num_ignored_.size() > 0) {
      fmt::print("LOG SUMMARY: suppressed, ");
      for (auto [kind, count] : num_ignored_) {
        const fmt::color level_color =
            sLogKindColorTable[static_cast<int>(kind)];
        fmt::print(
            "{}x {}   ", fmt::format(fg(level_color), "{}", kind), count);
      }
      println("");
    }
  }

  void setVerbosity(Log::Kind severity_to_print, bool unique_only) override
  {
    min_severity_ = severity_to_print;
    unique_only_ = unique_only;
  }

  void logImpl(
      Kind level, const char* sFile, const char* sFunction, const int nLine,
      const char* assertion_statement, const std::string& description) override
  {
    // This could hurt performance if we're really spamming log output, but it
    // will ensure that lines aren't intermingled between threads and that we
    // don't corrupt the maps during insertion.
    std::lock_guard<std::mutex> lk(log_mutex_);

    const auto last_err = last_error_for_line_.find({sFile, nLine});
    const bool already_printed = unique_only_ &&
                                 last_err != last_error_for_line_.end() &&
                                 (last_err->second.description == description);

    if (!already_printed && level >= min_severity_) {
      const char* level_text = sLogKindNameTable[static_cast<int>(level)];
      const fmt::color level_color =
          sLogKindColorTable[static_cast<int>(level)];

      println(
          "[{} {}]\n{}",
          fmt::format(fmt::emphasis::bold | fg(level_color), level_text),
          fmt::format(
              fmt::emphasis::faint | fg(level_color), "{}:{}", sFile, nLine),
          fmt::format(
              fg(level_color), ">{} {}", assertion_statement, description));
    } else {
      ++num_ignored_[level];
    }

    last_error_for_line_[{sFile, nLine}] = {
        .level = level,
        .description = description,
        .time = std::chrono::system_clock::now()};
  }

  struct Entry {
    Log::Kind level;
    std::string description;
    std::chrono::time_point<std::chrono::system_clock> time;
  };

  Log::Kind min_severity_ = Log::Kind::Debug;
  bool unique_only_ = true;
  bool print_summary_on_close_ = true;

  std::map<LogKey, Entry> last_error_for_line_;
  std::map<Log::Kind, int> num_ignored_;
  std::mutex log_mutex_;
};

Log& Log::instance()
{
  static LoggingImpl log;
  return log;
}

}  // namespace pangolin
