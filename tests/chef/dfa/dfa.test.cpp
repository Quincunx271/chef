#include <chef/dfa/dfa.hpp>

#include <catch2/catch.hpp>

TEST_CASE("dfa works")
{
    auto dfa = chef::dfa(4, 2,
        {
            {.from = 0, .to = 1, .on = 0},
            {.from = 0, .to = 0, .on = 1},
            {.from = 1, .to = 0, .on = 0},
            {.from = 1, .to = 2, .on = 1},
            {.from = 2, .to = 0, .on = 0},
            {.from = 2, .to = 3, .on = 1},
            {.from = 3, .to = 1, .on = 0},
            {.from = 3, .to = 0, .on = 1},
        });

    CHECK(dfa.num_states() == 4);
    CHECK(dfa.process(0, 0) == 1);
    CHECK(dfa.process(0, 1) == 0);
    CHECK(dfa.process(1, 0) == 0);
    CHECK(dfa.process(1, 1) == 2);
    CHECK(dfa.process(2, 0) == 0);
    CHECK(dfa.process(2, 1) == 3);
    CHECK(dfa.process(3, 0) == 1);
    CHECK(dfa.process(3, 1) == 0);
}
