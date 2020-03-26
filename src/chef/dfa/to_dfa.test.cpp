#include "./to_dfa.hpp"

#include <catch2/catch.hpp>

TEST_CASE("Powerset construction")
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

    auto dfa = chef::powerset_construction2<>(nfa);
}
