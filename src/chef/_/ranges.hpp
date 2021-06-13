#pragma once

#include <ranges>

#include <tl/tl.hpp>

#include <chef/_/fwd.hpp>

namespace chef::detail {
	template <typename Container, typename Range, typename Rng = std::remove_cvref_t<Range>>
		requires std::ranges::range<Rng> //
			Container to_container(Range&& rng)
		{
			using namespace std::ranges;

			Container c;
			if constexpr (sized_range<Rng> && requires { c.reserve(std::size_t(1)); }) {
				c.reserve(size(rng));
			}
			for (auto&& x : CHEF_FWD(rng)) {
				c.insert(c.end(), CHEF_FWD(x));
			}

			return c;
		}

	template <template <typename> class ContainerT, typename Range,
		typename Rng = std::remove_cvref_t<Range>>
		requires std::ranges::range<Rng>
	auto to_container(Range&& rng)
	{
		using Container = ContainerT<std::ranges::range_value_t<Rng>>;
		return detail::to_container<Container>(CHEF_FWD(rng));
	}

	template <typename IndexType = std::size_t>
	auto indices(std::ranges::sized_range auto const& rng)
	{
		return std::ranges::views::iota(
			IndexType(0), static_cast<IndexType>(std::ranges::size(rng)));
	}
}
