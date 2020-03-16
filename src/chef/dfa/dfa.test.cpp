#include "./dfa.hpp"

#include <catch2/catch.hpp>

TEST_CASE("Simple dfa usage")
{
    using state_t = chef::dfa::state_type;
    auto dfa = chef::dfa(std::vector<state_t> {0, 1, 2},
        std::vector<state_t> {
            // clang-format off
            /* from ; on: 0   1   2 */
            /*   0  */    0,  0,  2,
            /*   1  */    0,  2,  1,
            /*   2  */    2,  1,  0,
            // clang-format on
        },
        0, std::vector<state_t> {2});

    CHECK(dfa.num_states() == 3);
    CHECK(dfa.num_symbols() == 3);
    CHECK(dfa.start_state() == 0);
    CHECK(dfa.final_states() == std::vector<state_t> {2});

    CHECK_FALSE(dfa.is_final_state(0));
    CHECK_FALSE(dfa.is_final_state(1));
    CHECK(dfa.is_final_state(2));

    CHECK(dfa.compute_next(0, 0) == 0);
    CHECK(dfa.compute_next(0, 1) == 0);
    CHECK(dfa.compute_next(0, 2) == 2);
    CHECK(dfa.compute_next(1, 0) == 0);
    CHECK(dfa.compute_next(1, 1) == 2);
    CHECK(dfa.compute_next(1, 2) == 1);
    CHECK(dfa.compute_next(2, 0) == 2);
    CHECK(dfa.compute_next(2, 1) == 1);
    CHECK(dfa.compute_next(2, 2) == 0);
}
