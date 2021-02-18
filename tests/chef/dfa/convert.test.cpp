#include <chef/dfa/convert.hpp>

#include <ranges>

#include "chef/matchers.test.hpp"

#include <catch2/catch.hpp>

using IsPermutationOfVector = IsPermutation<std::vector<chef::state_type>>;
using ContainsSet = Contains<std::unordered_set<chef::state_type>>;

namespace {
	auto to_vector(std::ranges::range auto&& range)
	{
		using range_type = std::remove_cvref_t<decltype(range)>;
		return std::vector<std::ranges::range_value_t<range_type>>(
			std::ranges::begin(range), std::ranges::end(range));
	}
}

TEST_CASE("nfa -> dfa conversion works")
{
	auto nfa = chef::nfa(4, 2,
		{
			{.from = 0, .to = 1, .on = 0},
			{.from = 0, .to = 0, .on = 0},
			{.from = 0, .to = 0, .on = 1},
			{.from = 1, .to = 2, .on = 0},
			{.from = 2, .to = 3, .on = 1},
		});

	auto [dfa, categories] = chef::to_dfa(nfa,
		{
			{0},
			{1},
			{2},
			{3},
			{1, 2},
			{2, 3},
		});

	CHECK(dfa.num_symbols() == nfa.num_symbols());

	// Found by manually doing the conversion:
	CHECK(dfa.num_states() == 4);
	chef::state_type const st0 = 0;
	chef::state_type const st01 = dfa.process(st0, 0);
	chef::state_type const st012 = dfa.process(st01, 0);
	chef::state_type const st03 = dfa.process(st012, 1);
	CHECK_THAT(::to_vector(std::set({st0, st01, st012, st03})),
		IsPermutationOfVector({st0, st01, st012, st03}));

	// Redundant checks are commented out:
	// CHECK(dfa.process(st0, 0) == st01);
	CHECK(dfa.process(st0, 1) == st0);
	// CHECK(dfa.process(st01, 0) == st012);
	CHECK(dfa.process(st01, 1) == st0);
	CHECK(dfa.process(st012, 0) == st012);
	// CHECK(dfa.process(st012, 1) == st03);
	CHECK(dfa.process(st03, 0) == st01);
	CHECK(dfa.process(st03, 1) == st0);

	REQUIRE(categories.size() == 6);
	CHECK_THAT(::to_vector(categories[0]), IsPermutationOfVector({st0, st01, st012, st03}));
	CHECK_THAT(::to_vector(categories[1]), IsPermutationOfVector({st01, st012}));
	CHECK_THAT(::to_vector(categories[2]), IsPermutationOfVector({st012}));
	CHECK_THAT(::to_vector(categories[3]), IsPermutationOfVector({st03}));
	// 4: {1, 2}
	// 5: {2, 3}
	CHECK_THAT(::to_vector(categories[4]), IsPermutationOfVector({st01, st012}));
	CHECK_THAT(::to_vector(categories[5]), IsPermutationOfVector({st012, st03}));
}
