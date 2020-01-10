#include "./epsilon.hpp"

#include <catch2/catch.hpp>

TEST_CASE("epsilon comparison")
{
    CHECK(chef::epsilon == chef::epsilon);
    CHECK_FALSE(chef::epsilon != chef::epsilon);
    CHECK(
        std::hash<chef::epsilon_t>{}(chef::epsilon) == std::hash<chef::epsilon_t>{}(chef::epsilon));
}
