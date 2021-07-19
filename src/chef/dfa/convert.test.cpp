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
	auto nfa = chef::nfa(4, 3,
		{
			{.from = 0, .to = 1, .on = 1},
			{.from = 0, .to = 0, .on = 1},
			{.from = 0, .to = 0, .on = 2},
			{.from = 1, .to = 2, .on = 1},
			{.from = 2, .to = 3, .on = 2},
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

	CHECK(dfa.num_symbols() == nfa.num_symbols() - 1);

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

TEST_CASE("nfa -> dfa conversion works with epsilons")
{
	// NFA example from wikipedia's article on the powerset construction,
	// except accept states are much fewer.
	auto nfa = chef::nfa(4, 3,
		{
			{.from = 0, .to = 2, .on = chef::nfa::eps},
			{.from = 0, .to = 1, .on = 1},
			{.from = 1, .to = 1, .on = 2},
			{.from = 1, .to = 3, .on = 2},
			{.from = 2, .to = 1, .on = chef::nfa::eps},
			{.from = 2, .to = 3, .on = 1},
			{.from = 3, .to = 2, .on = 1},
		});

	auto [dfa, categories] = chef::to_dfa(nfa,
		{
			{3},
		});

	CHECK(dfa.num_symbols() == nfa.num_symbols() - 1);

	CHECK(dfa.num_states() == 5);
	chef::state_type const st123 = 0;
	chef::state_type const st24 = dfa.process(st123, 0);
	chef::state_type const st23 = dfa.process(st24, 0);
	chef::state_type const st4 = dfa.process(st23, 0);
	chef::state_type const st5 = dfa.process(st4, 1);
	CHECK_THAT(::to_vector(std::set({st123, st24, st23, st4, st5})),
		IsPermutationOfVector({st123, st24, st23, st4, st5}));

	// Redundant checks are commented out:
	// CHECK(dfa.process(st123, 0) == st24);
	CHECK(dfa.process(st123, 1) == st24);
	// CHECK(dfa.process(st24, 0) == st23);
	CHECK(dfa.process(st24, 1) == st24);
	// CHECK(dfa.process(st23, 0) == st4);
	CHECK(dfa.process(st23, 1) == st24);
	CHECK(dfa.process(st4, 0) == st23);
	// CHECK(dfa.process(st4, 1) == st5);
	CHECK(dfa.process(st5, 0) == st5);
	CHECK(dfa.process(st5, 1) == st5);

	REQUIRE(categories.size() == 1);
	CHECK_THAT(::to_vector(categories[0]), IsPermutationOfVector({st24, st4}));
}
