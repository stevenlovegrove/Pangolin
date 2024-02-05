#pragma once

#include <fmt/color.h>
#include <fmt/format.h>

#include <iostream>
#include <optional>

namespace pangolin
{

// Wraps fmt::format to conform to its compile-time checked APIs.
//
// With more recent versions of fmt and more recent compilers,
// more compile-time checking is enabled in fmt by default.
//
// Pangolin logging currently relies on some runtime
// format string construction, so we add a layer of indirection here.
//
// Alternatively we could eliminate the use of runtime format strings in Pangolin.
template <typename TFmt, typename... TArgs>
std::string format(TFmt&& fmt, TArgs&&... args) {
  if constexpr (std::is_same_v<TFmt, fmt::format_string<TArgs...>>) {
    // The type of the fmt string is known at compile-time, forward as-is
    return fmt::format(fmt, std::forward<TArgs>(args)...);
  } else if constexpr (std::is_convertible_v<TFmt, std::string_view>) {
    // The type of the fmt string is not known at compile-time, wrap in fmt::runtime
    return fmt::format(fmt::runtime(fmt), std::forward<TArgs>(args)...);
  } else {
    // The fmt string is some other type, like a `fmt::text_style`, forward as-is
    return fmt::format(fmt, std::forward<TArgs>(args)...);
  }
}

template <typename... TArgs>
void println(TArgs&&... args)
{
  fmt::print(fmt::runtime(format(std::forward<TArgs>(args)...)));
  std::cout << std::endl;
}

}  // namespace pangolin

// Format std::optional
namespace fmt
{
template <typename T>
struct formatter<std::optional<T>> {
  std::string_view underlying_fmt = "{}";
  std::string_view or_else = "<nullopt>";

  constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
  {
    return ctx.end();
  }

  template <typename FormatContext>
  auto format(const std::optional<T>& p, FormatContext& ctx) const
      -> decltype(ctx.out())
  {
    if (p.has_value()) {
      return vformat_to(
          ctx.out(), underlying_fmt, format_arg_store<FormatContext, T>{*p});
    } else {
      return format_to(ctx.out(), "{}", or_else);
    }
  }
};
}  // namespace fmt
