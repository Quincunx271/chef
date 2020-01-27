#include "./nfa.hpp"

#include <sstream>

#include <catch2/catch.hpp>

TEST_CASE("Simple nfa usage")
{
    chef::nfa_builder nfa({
        {0, 1, 1},
        {0, 1, 2},
        {0, 2, 0},
        {0, 0, 2},
        {1, 1, 1},
        {1, 1, 2},
        {1, 2, 0},
    });

    CHECK(nfa.num_symbols() == 3);
    CHECK(nfa.num_states() == 3);

    std::ostringstream out;
    // out << nfa;
    CHECK(out.str() == "");
}

TEST_CASE("nfa builder to value")
{
    auto const builder = chef::nfa_builder({
        {0, 1, 1},
        {0, 1, 2},
        {0, 2, 0},
        {0, 0, 2},
        {1, 1, 1},
        {1, 1, 2},
        {1, 2, 0},
    });
    auto const nfa = chef::nfa(builder);

    std::ostringstream nfa_out;
    nfa_out << nfa;

    std::ostringstream builder_out;
    builder_out << builder;
    CHECK(nfa_out.str() == builder_out.str());
}
