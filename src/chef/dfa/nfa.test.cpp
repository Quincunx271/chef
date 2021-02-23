#include <chef/dfa/nfa.hpp>

#include "chef/matchers.test.hpp"

#include <catch2/catch.hpp>

TEST_CASE("nfa works")
{
	auto nfa = chef::nfa(4, 2,
		{
			{.from = 0, .to = 1, .on = 0},
			{.from = 0, .to = 0, .on = 0},
			{.from = 0, .to = 0, .on = 1},
			{.from = 1, .to = 2, .on = 0},
			{.from = 2, .to = 3, .on = 1},
		});

	using IsPermutationOfSpan = IsPermutation<std::span<chef::state_type const>>;

	CHECK(nfa.num_states() == 4);
	CHECK_THAT(nfa.process(0, 0), IsPermutationOfSpan({0, 1}));
	CHECK_THAT(nfa.process(0, 1), IsPermutationOfSpan({0}));
	CHECK_THAT(nfa.process(1, 0), IsPermutationOfSpan({2}));
	CHECK_THAT(nfa.process(1, 1), IsPermutationOfSpan({}));
	CHECK_THAT(nfa.process(2, 0), IsPermutationOfSpan({}));
	CHECK_THAT(nfa.process(2, 1), IsPermutationOfSpan({3}));
	CHECK_THAT(nfa.process(3, 0), IsPermutationOfSpan({}));
	CHECK_THAT(nfa.process(3, 1), IsPermutationOfSpan({}));
}
