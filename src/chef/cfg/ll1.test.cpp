#include "./ll1.hpp"

#include <algorithm>
#include <iterator>
#include <ranges>
#include <sstream>
#include <stack>
#include <utility>
#include <vector>

#include <tl/tl.hpp>

#include <catch2/catch.hpp>

using chef::cfg_epsilon;
using chef::cfg_seq;
using chef::cfg_token;
using chef::cfg_var;
using chef::ll1_table;

namespace views = std::ranges::views;

using namespace chef::literals;

TEST_CASE("ll1_table::expand_variable accepts types it should")
{
	const ll1_table table({});

	SECTION("vector is allowed")
	{
		std::vector<cfg_seq::value_type> vector;
		STATIC_REQUIRE(requires { table.expand_variable(vector, ""_var, cfg_epsilon); });
	}
}

TEST_CASE("ll1_table::expand_variable works")
{
	const cfg_seq seq({1_tok, "B"_var, 2_tok});
	const ll1_table table({
		{"A"_var, {{0_tok, seq}}},
	});

	std::vector<cfg_seq::value_type> stack;
	table.expand_variable(stack, "A"_var, 0_tok);
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
			"S"_var,
			{
				{0_tok, cfg_seq({"F"_var})},
				{1_tok, cfg_seq({1_tok, "S"_var, 3_tok, "F"_var, 2_tok})},
			},
		},
		{"F"_var, {{0_tok, cfg_seq({0_tok})}}},
	});

	SECTION("accepts the types it should")
	{
		SECTION("accepts input ranges")
		{
			std::istringstream iss;
			auto input
				= std::ranges::istream_view<int>(iss) | views::transform([] TL(cfg_token(_1)));

			STATIC_REQUIRE(requires { table.parse("S"_var, input); });
		}
		SECTION("accepts std::vector<cfg_token>")
		{
			std::vector<cfg_token> input;

			STATIC_REQUIRE(requires { table.parse("S"_var, input); });
			STATIC_REQUIRE(requires { table.parse("S"_var, std::as_const(input)); });
		}
	}

	SECTION("parses valid input")
	{
		// Input: (a + a); from same example as the grammar.
		const std::vector<cfg_token> input{1_tok, 0_tok, 3_tok, 0_tok, 2_tok};

		CHECK(table.parse("S"_var, input));
	}
	SECTION("rejects invalid input")
	{
		// Input: (a + a; from same example as the grammar.

		// To produce a std::input_range, use istringstream + view::istream().
		const std::vector<cfg_token> input{1_tok, 0_tok, 3_tok, 0_tok};

		CHECK_FALSE(table.parse("S"_var, input));
	}
}

TEST_CASE("ll1_table from CFG works")
{
	SECTION("tiny CFG")
	{
		const chef::cfg tiny{
			"Start"_var,
			{
				{"Start"_var, {{{{0_tok}}}}},
			},
		};

		const ll1_table ll1(tiny);
		CHECK(ll1.parse("Start"_var, std::vector<cfg_token>{0_tok}));
		CHECK_FALSE(ll1.parse("Start"_var, std::vector<cfg_token>{1_tok}));
		CHECK_FALSE(ll1.parse("Start"_var, std::vector<cfg_token>{0_tok, 1_tok}));
		CHECK_FALSE(ll1.parse("Start"_var, std::vector<cfg_token>{}));
	}
}
