#include "./ll1.hpp"

#include <stack>
#include <vector>

#include <catch2/catch.hpp>

using chef::cfg_epsilon;
using chef::cfg_seq;
using chef::cfg_token;
using chef::cfg_var;
using chef::ll1_table;

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
	CHECK(cfg_seq(stack) == seq);
}
