#pragma once

#include <ranges>
#include <vector>

#include <catch2/catch.hpp>

namespace {
	template <std::ranges::range Range>
	class IsPermutation : public Catch::MatcherBase<Range> {
	private:
		using T = std::ranges::range_value_t<Range>;
		std::vector<T> target;

	public:
		explicit IsPermutation(std::vector<T> const& target)
			: target(target)
		{ }

		bool match(Range const& range) const override
		{
			if constexpr (std::ranges::sized_range<Range>) {
				if (target.size() != range.size()) {
					return false;
				}
			}
			return std::is_permutation(
				target.begin(), target.end(), std::ranges::begin(range), std::ranges::end(range));
		}

		std::string describe() const override
		{
			return "IsPermutation: " + ::Catch::Detail::stringify(target);
		}
	};
}
