#include "./filter.hpp"

#include <vector>

#include <chef/_/lambda.hpp>

#include <catch2/catch.hpp>

TEST_CASE("filter works")
{
    auto const vec = std::vector{1, 2, 3, 4, 5, 6};
    auto result = chef::_filter(vec, [] CHEF_I_L(it % 2 == 0));
    auto rvec = std::vector(result.begin(), result.iend());

    CHECK(rvec == std::vector{2, 4, 6});
}

TEST_CASE("filter works if always false")
{
    auto const vec = std::vector{1, 2, 3, 4, 5, 6};
    auto result = chef::_filter(vec, [] CHEF_I_L(false));
    auto rvec = std::vector(result.begin(), result.iend());

    CHECK(rvec == std::vector<int>{});
}

TEST_CASE("filter works if always true")
{
    auto const vec = std::vector{1, 2, 3, 4, 5, 6};
    auto result = chef::_filter(vec, [] CHEF_I_L(true));
    auto rvec = std::vector(result.begin(), result.iend());

    CHECK(rvec == vec);
}
