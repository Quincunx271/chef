#include <chef/cfg/cfg.hpp>

#include <catch2/catch.hpp>
#include <chef/matchers.test.inl>

using FirstSetContains = Contains<std::set<chef::cfg_token>>;

using namespace chef::literals;

TEST_CASE("CFG works for simple grammar")
{
	// Sample CFG comes from https://youtu.be/vrWr_5Yk1OA?t=2187
	// 0 == b
	// 1 == a
	chef::cfg cfg{
		"Start"_var,
		{
			{"Start"_var,
				{{
					{{"A"_var}},
					{{"B"_var}},
				}}},
			{"A"_var, {{{{0_tok}}}}},
			{"B"_var,
				{{
					{{"B"_var, "A"_var, "C"_var, 0_tok}},
					{{1_tok}},
				}}},
			{"C"_var,
				{{
					{{"A"_var, "D"_var}},
					{{chef::cfg_epsilon}},
				}}},
			{"D"_var,
				{{
					{{"B"_var, "C"_var}},
					{{0_tok, "C"_var}},
				}}},
		},
	};

	SECTION("First sets are computed properly")
	{
		std::map<chef::cfg_var, std::set<chef::cfg_token>> first_sets = chef::first_sets(cfg);
		CHECK(first_sets["Start"_var] == std::set{0_tok, 1_tok});
		CHECK(first_sets["A"_var] == std::set{0_tok});
		CHECK(first_sets["B"_var] == std::set{1_tok});
		CHECK(first_sets["C"_var] == std::set{0_tok, chef::cfg_epsilon});
		CHECK(first_sets["D"_var] == std::set{0_tok, 1_tok});
	}
	SECTION("Follow sets are computed properly")
	{
		const std::map<chef::cfg_var, std::set<chef::cfg_token>> first_sets = chef::first_sets(cfg);
		std::map<chef::cfg_var, std::set<chef::cfg_token>> follow_sets
			= chef::follow_sets(cfg, first_sets);
		CHECK(follow_sets["Start"_var] == std::set{chef::cfg_eof});
		CHECK(follow_sets["A"_var] == std::set{chef::cfg_eof, 0_tok, 1_tok});
		CHECK(follow_sets["B"_var] == std::set{chef::cfg_eof, 0_tok});
		CHECK(follow_sets["C"_var] == std::set{0_tok});
		CHECK(follow_sets["D"_var] == std::set{0_tok});
	}
}

TEST_CASE("First sets don't contain epsilon if the symbol cannot be fully erased")
{
	chef::cfg cfg{
		"S"_var,
		{
			{"S"_var,
				{{
					{{"A"_var, "B"_var}},
				}}},
			{"A"_var,
				{{
					{{"C"_var}},
					{{chef::cfg_epsilon}},
				}}},
			{"B"_var,
				{{
					{{0_tok}},
				}}},
			{"C"_var,
				{{
					{{"D"_var, 0_tok}},
				}}},
			{"D"_var,
				{{
					{{chef::cfg_epsilon}},
				}}},
		},
	};

	std::map<chef::cfg_var, std::set<chef::cfg_token>> first_sets = chef::first_sets(cfg);

	CHECK_THAT(first_sets["D"_var], FirstSetContains(chef::cfg_epsilon));
	CHECK_THAT(first_sets["C"_var], !FirstSetContains(chef::cfg_epsilon));
	CHECK_THAT(first_sets["S"_var], !FirstSetContains(chef::cfg_epsilon));
}
