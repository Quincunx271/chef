#include <chef/cfg/cfg.hpp>

#include <catch2/catch.hpp>
#include <chef/matchers.test.inl>

using FirstSetContains = Contains<std::set<chef::cfg_token>>;

TEST_CASE("CFG works for simple grammar")
{
	// Sample CFG comes from https://youtu.be/vrWr_5Yk1OA?t=2187
	chef::cfg cfg{
		{
			{chef::cfg_var("Start"),
				{{
					{{chef::cfg_var("A")}},
					{{chef::cfg_var("B")}},
				}}},
			{chef::cfg_var("A"), {{{{chef::cfg_token(0)}}}}},
			{chef::cfg_var("B"),
				{{
					{{chef::cfg_var("B"), chef::cfg_var("A"), chef::cfg_var("C"),
						chef::cfg_token(0)}},
					{{chef::cfg_token(1)}},
				}}},
			{chef::cfg_var("C"),
				{{
					{{chef::cfg_var("A"), chef::cfg_var("D")}},
					{{chef::cfg_epsilon}},
				}}},
			{chef::cfg_var("D"),
				{{
					{{chef::cfg_var("B"), chef::cfg_var("C")}},
					{{chef::cfg_token(0), chef::cfg_var("C")}},
				}}},
		},
		{chef::cfg_epsilon},
	};

	SECTION("Tokens are appropriately vanishable")
	{
		CHECK(cfg.is_vanishable(chef::cfg_epsilon));
		CHECK_FALSE(cfg.is_vanishable(chef::cfg_token(0)));
		CHECK_FALSE(cfg.is_vanishable(chef::cfg_token(1)));
	}

	SECTION("First sets are computed properly")
	{
		std::map<chef::cfg_var, std::set<chef::cfg_token>> first_sets = chef::first_sets(cfg);
		CHECK(
			first_sets[chef::cfg_var("Start")] == std::set{chef::cfg_token(0), chef::cfg_token(1)});
		CHECK(first_sets[chef::cfg_var("A")] == std::set{chef::cfg_token(0)});
		CHECK(first_sets[chef::cfg_var("B")] == std::set{chef::cfg_token(1)});
		CHECK(first_sets[chef::cfg_var("C")] == std::set{chef::cfg_token(0), chef::cfg_epsilon});
		CHECK(first_sets[chef::cfg_var("D")] == std::set{chef::cfg_token(0), chef::cfg_token(1)});
	}
}

TEST_CASE("First sets don't contain epsilon if the symbol cannot be fully erased")
{
	chef::cfg cfg{
		{
			{chef::cfg_var("S"),
				{{
					{{chef::cfg_var("A"), chef::cfg_var("B")}},
				}}},
			{chef::cfg_var("A"),
				{{
					{{chef::cfg_var("C")}},
					{{chef::cfg_epsilon}},
				}}},
			{chef::cfg_var("B"),
				{{
					{{chef::cfg_token(0)}},
				}}},
			{chef::cfg_var("C"),
				{{
					{{chef::cfg_var("D"), chef::cfg_token(0)}},
				}}},
			{chef::cfg_var("D"),
				{{
					{{chef::cfg_epsilon}},
				}}},
		},
		{chef::cfg_epsilon},
	};

	std::map<chef::cfg_var, std::set<chef::cfg_token>> first_sets = chef::first_sets(cfg);

	CHECK_THAT(first_sets[chef::cfg_var("D")], FirstSetContains(chef::cfg_epsilon));
	CHECK_THAT(first_sets[chef::cfg_var("C")], !FirstSetContains(chef::cfg_epsilon));
	CHECK_THAT(first_sets[chef::cfg_var("S")], !FirstSetContains(chef::cfg_epsilon));
}
