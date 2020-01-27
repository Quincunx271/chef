#include "./nfa.hpp"

#include <sstream>

#include <catch2/catch.hpp>

TEST_CASE("Simple nfa usage")
{
    chef::nfa_builder nfa({
        {0, 1, 1},
        {0, 2, 0},
        {0, 0, 2},
        {1, 1, 1},
        {1, 1, 2},
        {1, 2, 0},
    });

    std::ostringstream out;
    // out << nfa;
    CHECK(out.str() == "");
}
