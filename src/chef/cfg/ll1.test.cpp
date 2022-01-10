#include "./ll1.hpp"

#include <algorithm>
#include <iterator>
#include <ranges>
#include <sstream>
#include <stack>
#include <vector>

#include <tl/tl.hpp>

#include <catch2/catch.hpp>

using chef::cfg_epsilon;
using chef::cfg_seq;
using chef::cfg_token;
using chef::cfg_var;
using chef::ll1_table;

namespace views = std::ranges::views;

TEST_CASE("ll1_table::expand_variable accepts types it should")
{
	const ll1_table table({});

	SECTION("vector is allowed")
	{
		std::vector<cfg_seq::value_type> vector;
		STATIC_REQUIRE(requires { table.expand_variable(vector, cfg_var(""), cfg_epsilon); });
	}
}

TEST_CASE("ll1_table::expand_variable works")
{
	const cfg_seq seq({cfg_token(1), cfg_var("B"), cfg_token(2)});
	const ll1_table table({
		{cfg_var("A"), {{cfg_token(0), seq}}},
	});

	std::vector<cfg_seq::value_type> stack;
	table.expand_variable(stack, cfg_var("A"), cfg_token(0));
	std::ranges::reverse(stack);
	CHECK(cfg_seq(stack) == seq);
}

TEST_CASE("ll1_table::parse works")
{
	// Example from https://en.wikipedia.org/wiki/LL_parser#Concrete_example.
	// S -> F | (S + F)
	// F -> a
	// 0: a
	// 1: (
	// 2: )
	// 3: +
	const ll1_table table({
		{
			cfg_var("S"),
			{
				{cfg_token(0), cfg_seq({cfg_var("F")})},
				{cfg_token(1),
					cfg_seq(
						{cfg_token(1), cfg_var("S"), cfg_token(3), cfg_var("F"), cfg_token(2)})},
			},
		},
		{cfg_var("F"), {{cfg_token(0), cfg_seq({cfg_token(0)})}}},
	});

	SECTION("parses valid input")
	{
		// Input: (a + a); from same example as the grammar.

		// To produce a std::input_range, use istringstream + view::istream().
		std::istringstream iss("1 0 3 0 2");
		auto input = std::ranges::istream_view<int>(iss) | views::transform([] TL(cfg_token(_1)));

		CHECK(table.parse(cfg_var("S"), input));
	}
	SECTION("rejects invalid input")
	{
		// Input: (a + a); from same example as the grammar.

		// To produce a std::input_range, use istringstream + view::istream().
		std::istringstream iss("1 0 3 0");
		auto input = std::ranges::istream_view<int>(iss) | views::transform([] TL(cfg_token(_1)));

		CHECK_FALSE(table.parse(cfg_var("S"), input));
	}
}
