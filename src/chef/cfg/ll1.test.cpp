#include "./ll1.hpp"

#include <algorithm>
#include <iterator>
#include <ranges>
#include <sstream>
#include <stack>
#include <utility>
#include <vector>

#include <tl/tl.hpp>

#include <chef/errors.hpp>

#include <catch2/catch.hpp>

using chef::cfg_epsilon;
using chef::cfg_seq;
using chef::cfg_token;
using chef::cfg_var;
using chef::ll1_table;
namespace rt_ast = chef::rt_ast;

using Catch::Matchers::Contains;

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
	SECTION("invalid grammars")
	{
		SECTION("not left-factored")
		{
			const chef::cfg grammar{"Start"_var,
				{{
					"Start"_var,
					{{
						{{0_tok, 1_tok}},
						{{0_tok, 0_tok}},
					}},
				}}};
			CHECK_THROWS_AS(ll1_table(grammar), chef::construction_error);
			CHECK_THROWS_WITH(ll1_table(grammar),
				Contains("Start") && Contains("LL(1)") && Contains("left factor"));
		}
		SECTION("has left recursion")
		{
			const chef::cfg grammar{"Start"_var,
				{{
					"Start"_var,
					{{
						{{"Start"_var, 0_tok}},
						{{0_tok}},
					}},
				}}};
			CHECK_THROWS_AS(ll1_table(grammar), chef::construction_error);
			CHECK_THROWS_WITH(ll1_table(grammar),
				// Left recursion may trigger the left factoring error, and that's okay.
				Contains("Start") && Contains("LL(1)"));
		}
	}
}

TEST_CASE("ll1_table::parse_rt works")
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

			STATIC_REQUIRE(requires { table.parse_rt("S"_var, input); });
		}
		SECTION("accepts std::vector<cfg_token>")
		{
			std::vector<cfg_token> input;

			STATIC_REQUIRE(requires { table.parse_rt("S"_var, input); });
			STATIC_REQUIRE(requires { table.parse_rt("S"_var, std::as_const(input)); });
		}
	}

	SECTION("parses valid input")
	{
		// Input: (a + a); from same example as the grammar.
		const std::vector<cfg_token> input{1_tok, 0_tok, 3_tok, 0_tok, 2_tok};

		std::optional<rt_ast::ast_node> result = table.parse_rt("S"_var, input);
		REQUIRE(result.has_value());
		CHECK(result->name == "S");
		REQUIRE(result->children.size() == 5);

		REQUIRE(std::holds_alternative<cfg_token>(result->children[0]));
		REQUIRE(std::holds_alternative<cfg_token>(result->children[2]));
		REQUIRE(std::holds_alternative<cfg_token>(result->children[4]));
		CHECK(std::get<cfg_token>(result->children[0]) == 1_tok);
		// CHECK(result->children[1] == "S");
		CHECK(std::get<cfg_token>(result->children[2]) == 3_tok);
		// CHECK(result->children[3] == "F");
		CHECK(std::get<cfg_token>(result->children[4]) == 2_tok);

		REQUIRE(std::holds_alternative<std::unique_ptr<rt_ast::ast_node>>(result->children[1]));
		REQUIRE(std::holds_alternative<std::unique_ptr<rt_ast::ast_node>>(result->children[3]));

		const auto& s_node = std::get<std::unique_ptr<rt_ast::ast_node>>(result->children[1]);
		CHECK(s_node->name == "S");
		REQUIRE(s_node->children.size() == 1);
		CHECK(std::holds_alternative<std::unique_ptr<rt_ast::ast_node>>(s_node->children[0]));
		const auto& s_f_node = std::get<std::unique_ptr<rt_ast::ast_node>>(s_node->children[0]);
		CHECK(s_f_node->name == "F");
		REQUIRE(s_f_node->children.size() == 1);
		REQUIRE(std::holds_alternative<cfg_token>(s_f_node->children[0]));
		CHECK(std::get<cfg_token>(s_f_node->children[0]) == 0_tok);

		const auto& f_node = std::get<std::unique_ptr<rt_ast::ast_node>>(result->children[3]);
		CHECK(f_node->name == "F");
		REQUIRE(f_node->children.size() == 1);
		REQUIRE(std::holds_alternative<cfg_token>(f_node->children[0]));
		CHECK(std::get<cfg_token>(f_node->children[0]) == 0_tok);
	}
	SECTION("rejects invalid input")
	{
		// Input: (a + a; from same example as the grammar.

		// To produce a std::input_range, use istringstream + view::istream().
		const std::vector<cfg_token> input{1_tok, 0_tok, 3_tok, 0_tok};

		CHECK_FALSE(table.parse_rt("S"_var, input).has_value());
	}
}
