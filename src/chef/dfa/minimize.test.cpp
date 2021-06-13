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

TEST_CASE("dfa minimization works")
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

TEST_CASE("dfa minimization works - Wikipedia example")
{
	// DFA from https://en.wikipedia.org/wiki/DFA_minimization
	auto const dfa_in = chef::dfa(6, 2,
		{
			{.from = 0, .to = 1, .on = 0},
			{.from = 0, .to = 2, .on = 1},
			{.from = 1, .to = 0, .on = 0},
			{.from = 1, .to = 3, .on = 1},
			{.from = 2, .to = 4, .on = 0},
			{.from = 2, .to = 5, .on = 1},
			{.from = 3, .to = 4, .on = 0},
			{.from = 3, .to = 5, .on = 1},
			{.from = 4, .to = 4, .on = 0},
			{.from = 4, .to = 5, .on = 1},
			{.from = 5, .to = 5, .on = 0},
			{.from = 5, .to = 5, .on = 1},
		});

	auto [dfa, categories] = chef::minimize(dfa_in, {{2, 3, 4}});

	CHECK(dfa.num_symbols() == 2);
	CHECK(dfa.num_states() == 3);

	chef::state_type const st0 = 0;
	chef::state_type const st1 = dfa.process(st0, 1);
	chef::state_type const st2 = dfa.process(st1, 1);
	CHECK_THAT(::to_vector(std::set({st0, st1, st2})), IsPermutationOfVector({st0, st1, st2}));

	// Redundant checks are commented out:
	CHECK(dfa.process(st0, 0) == st0);
	// CHECK(dfa.process(st0, 1) == st1);
	CHECK(dfa.process(st1, 0) == st1);
	// CHECK(dfa.process(st1, 1) == st2);
	CHECK(dfa.process(st2, 0) == st2);
	CHECK(dfa.process(st2, 1) == st2);

	REQUIRE(categories.size() == 1);
	CHECK_THAT(::to_vector(categories[0]), IsPermutationOfVector({st1}));
}

TEST_CASE("dfa minimization works - difference mod 4")
{
	// DFA capturing {n0(s) == n1(s) mod 4}, in "grid form"
	auto const dfa_in = chef::dfa(16, 2,
		{
			{.from = 0b00'00, .to = 0b00'01, .on = 0},
			{.from = 0b00'00, .to = 0b01'00, .on = 1},
			{.from = 0b00'01, .to = 0b00'10, .on = 0},
			{.from = 0b00'01, .to = 0b01'01, .on = 1},
			{.from = 0b00'10, .to = 0b00'11, .on = 0},
			{.from = 0b00'10, .to = 0b01'10, .on = 1},
			{.from = 0b00'11, .to = 0b00'00, .on = 0},
			{.from = 0b00'11, .to = 0b01'11, .on = 1},

			{.from = 0b01'00, .to = 0b01'01, .on = 0},
			{.from = 0b01'00, .to = 0b10'00, .on = 1},
			{.from = 0b01'01, .to = 0b01'10, .on = 0},
			{.from = 0b01'01, .to = 0b10'01, .on = 1},
			{.from = 0b01'10, .to = 0b01'11, .on = 0},
			{.from = 0b01'10, .to = 0b10'10, .on = 1},
			{.from = 0b01'11, .to = 0b01'00, .on = 0},
			{.from = 0b01'11, .to = 0b10'11, .on = 1},

			{.from = 0b10'00, .to = 0b10'01, .on = 0},
			{.from = 0b10'00, .to = 0b11'00, .on = 1},
			{.from = 0b10'01, .to = 0b10'10, .on = 0},
			{.from = 0b10'01, .to = 0b11'01, .on = 1},
			{.from = 0b10'10, .to = 0b10'11, .on = 0},
			{.from = 0b10'10, .to = 0b11'10, .on = 1},
			{.from = 0b10'11, .to = 0b10'00, .on = 0},
			{.from = 0b10'11, .to = 0b11'11, .on = 1},

			{.from = 0b11'00, .to = 0b11'01, .on = 0},
			{.from = 0b11'00, .to = 0b00'00, .on = 1},
			{.from = 0b11'01, .to = 0b11'10, .on = 0},
			{.from = 0b11'01, .to = 0b00'01, .on = 1},
			{.from = 0b11'10, .to = 0b11'11, .on = 0},
			{.from = 0b11'10, .to = 0b00'10, .on = 1},
			{.from = 0b11'11, .to = 0b11'00, .on = 0},
			{.from = 0b11'11, .to = 0b00'11, .on = 1},
		});

	auto [dfa, categories] = chef::minimize(dfa_in, {{0b00'00, 0b01'01, 0b10'10, 0b11'11}});

	// Minimized DFA: n0(s) - n1(s) == 0 mod 4. So only 4 states (remainders 0, 1, 2, 3)
	CHECK(dfa.num_symbols() == 2);
	CHECK(dfa.num_states() == 4);

	chef::state_type const st0 = 0;
	chef::state_type const st1 = dfa.process(st0, 1);
	chef::state_type const st2 = dfa.process(st1, 1);
	chef::state_type const st3 = dfa.process(st2, 1);
	CHECK_THAT(
		::to_vector(std::set({st0, st1, st2, st3})), IsPermutationOfVector({st0, st1, st2, st3}));

	CHECK(dfa.process(st3, 1) == st0); // Should wrap around.

	// Redundant checks are commented out:
	CHECK(dfa.process(st0, 0) == st3);
	CHECK(dfa.process(st0, 1) == st1);
	CHECK(dfa.process(st1, 0) == st0);
	CHECK(dfa.process(st1, 1) == st2);
	CHECK(dfa.process(st2, 0) == st1);
	CHECK(dfa.process(st2, 1) == st3);
	CHECK(dfa.process(st3, 0) == st2);
	CHECK(dfa.process(st3, 1) == st0);

	REQUIRE(categories.size() == 1);
	CHECK_THAT(::to_vector(categories[0]), IsPermutationOfVector({st0}));
}
