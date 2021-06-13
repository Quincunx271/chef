#include "./minimize.hpp"

#include <ranges>
#include <set>

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
	// Same DFA as in convert.test.cpp
	/*
	 () 1
	+---+         +---+
	| 0 | ---0--> | 1 |
	|   | <--1--- |   |
	+---+         +---+
	  ^          .^ |
	  |        ./   |
	 [1]     0/    [0]
	  |    ./       |
	  |  ./         v
	+---+         +---+
	| 3 |         | 2 | () 0
	|   | <--1--- |   |
	+---+         +---+
	*/
	auto const dfa_in = chef::dfa(4, 2,
		{
			{.from = 0, .to = 1, .on = 0},
			{.from = 0, .to = 0, .on = 1},
			{.from = 1, .to = 2, .on = 0},
			{.from = 1, .to = 0, .on = 1},
			{.from = 2, .to = 2, .on = 0},
			{.from = 2, .to = 3, .on = 1},
			{.from = 3, .to = 1, .on = 0},
			{.from = 3, .to = 0, .on = 1},
		});

	auto [dfa, categories] = chef::minimize(dfa_in, {{2}});

	CHECK(dfa.num_symbols() == 2);

	// Found by manually doing the conversion (mentally superimpose states 0 and 3):
	CHECK(dfa.num_states() == 3);
	chef::state_type const st0 = 0;
	chef::state_type const st1 = dfa.process(st0, 0);
	chef::state_type const st2 = dfa.process(st1, 0);
	chef::state_type const st3 = dfa.process(st2, 1);
	CHECK_THAT(::to_vector(std::set({st0, st1, st2, st3})), IsPermutationOfVector({st0, st1, st2}));

	// Redundant checks are commented out:
	// CHECK(dfa.process(st0, 0) == st1);
	CHECK(dfa.process(st0, 1) == st0);
	// CHECK(dfa.process(st1, 0) == st2);
	CHECK(dfa.process(st1, 1) == st0);
	CHECK(dfa.process(st2, 0) == st2);
	// CHECK(dfa.process(st2, 1) == st0);

	REQUIRE(categories.size() == 1);
	CHECK_THAT(::to_vector(categories[0]), IsPermutationOfVector({st2}));
}
