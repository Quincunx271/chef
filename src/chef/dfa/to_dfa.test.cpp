#include "./to_dfa.hpp"

#include <unordered_set>

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

    auto [dfa, mstate_info] = chef::powerset_construction2<>(nfa);
    std::unordered_set<chef::nfa::state_type> final_states{nfa.start_state()};
    mstate_info.states_matching()
}
