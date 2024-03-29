#pragma once

#include <algorithm>
#include <unordered_set>

#include <tl/tl.hpp>

#include <chef/_/ranges.hpp>

namespace chef::detail {
	template <typename T, typename... Rest>
	std::unordered_set<T, Rest...> intersect(
		std::unordered_set<T, Rest...> const& lhs, std::unordered_set<T, Rest...> const& rhs)
	{
		namespace views = std::ranges::views;
		return chef::detail::to_container<std::unordered_set<T, Rest...>>(
			lhs | views::filter([&](auto const& x) { return rhs.contains(x); }));
	}

	template <typename T, typename... Rest>
	bool has_intersect(
		std::unordered_set<T, Rest...> const& lhs, std::unordered_set<T, Rest...> const& rhs)
	{
		return std::any_of(lhs.begin(), lhs.end(), [&](auto const& x) { return rhs.contains(x); });
	}

	template <typename T, typename... Rest>
	std::unordered_set<T, Rest...> difference(
		std::unordered_set<T, Rest...> const& lhs, std::unordered_set<T, Rest...> const& rhs)
	{
		namespace views = std::ranges::views;
		return chef::detail::to_container<std::unordered_set<T, Rest...>>(
			lhs | views::filter([&](auto const& x) { return !rhs.contains(x); }));
	}

	template <typename T, typename... Rest>
	bool has_difference(
		std::unordered_set<T, Rest...> const& lhs, std::unordered_set<T, Rest...> const& rhs)
	{
		return std::any_of(lhs.begin(), lhs.end(), [&](auto const& x) { return !rhs.contains(x); });
	}
}
