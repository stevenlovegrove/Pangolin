#pragma once

#include <fmt/format.h>
#include <fmt/color.h>
#include <iostream>
#include <optional>

namespace pangolin
{

template<typename ...TArgs>
void println(TArgs&&... args)
{
    fmt::print(std::forward<TArgs>(args)...);
    std::cout << std::endl;
}

}

// Format std::optional
namespace fmt{
template <typename T>
struct formatter<std::optional<T>> {
	std::string_view underlying_fmt = "{}";
	std::string_view or_else = "<nullopt>";

	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.end();
	}

	template <typename FormatContext>
	auto format(const std::optional<T>& p, FormatContext& ctx) const -> decltype(ctx.out()) {
		if(p.has_value()) {
			return vformat_to(ctx.out(), underlying_fmt, format_arg_store<FormatContext, T>{*p});
		} else {
			return format_to(ctx.out(), "{}", or_else);
		}
	}
};
}
