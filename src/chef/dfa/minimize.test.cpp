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

TEST_CASE("dfa minimization works - sum mod 5 with many symbols")
{
	// L = {sum(d for d in s) == 3 mod 5}.
	// S = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}
	// Inefficient DFA to capture this: sum each kind of digit individually, then combine.
	// 12-dimensional "grid".
	auto const p1 = [](unsigned int n) { return (n + 1) % 5; };
	std::vector<chef::fa_edge> edges;
	std::unordered_set<chef::state_type> finals;

	for (unsigned int ones = 0; ones < 5; ++ones) {
		for (unsigned int twos = 0; twos < 5; ++twos) {
			for (unsigned int threes = 0; threes < 5; ++threes) {
				for (unsigned int fours = 0; fours < 5; ++fours) {
					if ((ones * 1 + twos * 2 + threes * 3 + fours * 4) % 5 == 3)
						finals.insert((((((ones * 5) + twos) * 5) + threes) * 5) + fours);

					for (chef::symbol_type const zero : std::vector<chef::symbol_type>{0, 5, 10})
						edges.push_back(chef::fa_edge{
							.from = (((((ones * 5) + twos) * 5) + threes) * 5) + fours,
							.to = (((((ones * 5) + twos) * 5) + threes) * 5) + fours,
							.on = zero,
						});
					for (chef::symbol_type const one : std::vector<chef::symbol_type>{1, 6, 11})
						edges.push_back(chef::fa_edge{
							.from = (((((ones * 5) + twos) * 5) + threes) * 5) + fours,
							.to = (((((p1(ones) * 5) + twos) * 5) + threes) * 5) + fours,
							.on = one,
						});
					for (chef::symbol_type const two : std::vector<chef::symbol_type>{2, 7, 12})
						edges.push_back(chef::fa_edge{
							.from = (((((ones * 5) + twos) * 5) + threes) * 5) + fours,
							.to = (((((ones * 5) + p1(twos)) * 5) + threes) * 5) + fours,
							.on = two,
						});
					for (chef::symbol_type const three : std::vector<chef::symbol_type>{3, 8})
						edges.push_back(chef::fa_edge{
							.from = (((((ones * 5) + twos) * 5) + threes) * 5) + fours,
							.to = (((((ones * 5) + twos) * 5) + p1(threes)) * 5) + fours,
							.on = three,
						});
					for (chef::symbol_type const four : std::vector<chef::symbol_type>{4, 9})
						edges.push_back(chef::fa_edge{
							.from = (((((ones * 5) + twos) * 5) + threes) * 5) + fours,
							.to = (((((ones * 5) + twos) * 5) + threes) * 5) + p1(fours),
							.on = four,
						});
				}
			}
		}
	}
	// Num states: 5**4
	auto const dfa_in = chef::dfa(625, 13, edges);

	auto [dfa, categories] = chef::minimize(dfa_in, {finals});

	// Minimized DFA: we're working mod 5, so only 5 states.
	CHECK(dfa.num_symbols() == 13);
	CHECK(dfa.num_states() == 5);

	chef::state_type const st0 = 0;
	chef::state_type const st1 = dfa.process(st0, 1);
	chef::state_type const st2 = dfa.process(st1, 1);
	chef::state_type const st3 = dfa.process(st2, 1);
	chef::state_type const st4 = dfa.process(st3, 1);
	CHECK_THAT(::to_vector(dfa.states()), IsPermutationOfVector({st0, st1, st2, st3, st4}));

	CHECK(dfa.process(st4, 1) == st0); // Should wrap around.

	chef::state_type const sts[] = {st0, st1, st2, st3, st4};
	for (std::size_t const from_index : chef::detail::indices(sts)) {
		chef::state_type const from = sts[from_index];

		for (chef::symbol_type const sym : dfa.symbols()) {
			chef::state_type const to = dfa.process(from, sym);
			std::size_t const to_index
				= std::distance(std::begin(sts), std::find(std::begin(sts), std::end(sts), to));

			CAPTURE(from_index, to_index, std::size_t(sym));
			CHECK((from_index + sym) % 5 == to_index);
		}
	}

	REQUIRE(categories.size() == 1);
	CHECK_THAT(::to_vector(categories[0]), IsPermutationOfVector({st3}));
}
