#include "./ranges.hpp"

#include <numeric>
#include <ranges>
#include <unordered_set>
#include <vector>

#include <tl/tl.hpp>

#include <catch2/catch.hpp>

namespace views = std::ranges::views;
using namespace Catch::Matchers;

TEST_CASE("to_container works for sized range && reservable container")
{
	SECTION("vector")
	{
		auto const bound = GENERATE(values({0, 1, 2, 3, 8, 9, 10, 13, 17, 19, 23}));

		auto const range = views::iota(0, bound);
		static_assert(std::ranges::sized_range<decltype(range)>);

		auto const vec = chef::detail::to_container<std::vector>(range);
		static_assert(std::same_as<std::remove_cvref_t<decltype(vec)>, std::vector<int>>);
		auto const vec2 = chef::detail::to_container<std::vector<int>>(range);
		static_assert(std::same_as<std::remove_cvref_t<decltype(vec2)>, std::vector<int>>);

		std::vector<int> expected(bound);
		std::iota(expected.begin(), expected.end(), 0);

		CHECK_THAT(vec, Equals(expected));
		CHECK_THAT(vec2, Equals(expected));

		CHECK(vec == vec2);
		CHECK(vec.capacity() == vec.size());
		CHECK(vec2.capacity() == vec2.size());
	}
	SECTION("unordered_set")
	{
		auto const bound = GENERATE(values({0, 1, 2, 3, 8, 9, 10, 13, 17, 19, 23}));

		auto const range = views::iota(0, bound);
		auto const set = chef::detail::to_container<std::unordered_set>(range);
		static_assert(std::same_as<std::remove_cvref_t<decltype(set)>, std::unordered_set<int>>);
		auto const set2 = chef::detail::to_container<std::unordered_set<int>>(range);
		static_assert(std::same_as<std::remove_cvref_t<decltype(set2)>, std::unordered_set<int>>);

		std::vector<int> expected(bound);
		std::iota(expected.begin(), expected.end(), 0);

		std::vector<int> set_cpy(set.begin(), set.end());
		std::ranges::sort(set_cpy);

		std::vector<int> set2_cpy(set2.begin(), set2.end());
		std::ranges::sort(set2_cpy);

		CHECK_THAT(set_cpy, Equals(expected));
		CHECK_THAT(set2_cpy, Equals(expected));

		CHECK(set == set2);
	}
}

TEST_CASE("to_container works for unsized range")
{
	SECTION("vector")
	{
		auto const bound = GENERATE(values({0, 1, 10, 23}));

		auto const range
			= [=] { return views::iota(0, bound) | views::filter([] TL(_1 % 2 == 1)); };
		auto const vec = chef::detail::to_container<std::vector>(range());
		static_assert(std::same_as<std::remove_cvref_t<decltype(vec)>, std::vector<int>>);
		auto const vec2 = chef::detail::to_container<std::vector<int>>(range());
		static_assert(std::same_as<std::remove_cvref_t<decltype(vec2)>, std::vector<int>>);

		std::vector<int> expected(bound);
		std::iota(expected.begin(), expected.end(), 0);
		std::erase_if(expected, [] TL(_1 % 2 != 1));

		CHECK_THAT(vec, Equals(expected));
		CHECK_THAT(vec2, Equals(expected));

		CHECK(vec == vec2);
	}
	SECTION("unordered_set")
	{
		auto const bound = GENERATE(values({0, 1, 10, 23}));

		auto const range
			= [=] { return views::iota(0, bound) | views::filter([] TL(_1 % 2 == 1)); };
		auto const set = chef::detail::to_container<std::unordered_set>(range());
		static_assert(std::same_as<std::remove_cvref_t<decltype(set)>, std::unordered_set<int>>);
		auto const set2 = chef::detail::to_container<std::unordered_set<int>>(range());
		static_assert(std::same_as<std::remove_cvref_t<decltype(set2)>, std::unordered_set<int>>);

		std::vector<int> expected(bound);
		std::iota(expected.begin(), expected.end(), 0);
		std::erase_if(expected, [] TL(_1 % 2 != 1));

		std::vector<int> set_cpy(set.begin(), set.end());
		std::ranges::sort(set_cpy);

		std::vector<int> set2_cpy(set2.begin(), set2.end());
		std::ranges::sort(set2_cpy);

		CHECK_THAT(set_cpy, Equals(expected));
		CHECK_THAT(set2_cpy, Equals(expected));

		CHECK(set == set2);
	}
}
