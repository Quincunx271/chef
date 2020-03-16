#include "./simple_range.hpp"

#include <vector>

#include <chef/_/function_objects.hpp>

#include <catch2/catch.hpp>

namespace {
    TEST_CASE("simple range basic")
    {
        auto range = chef::_simple_range(0, chef::_identity,
            [](int state) -> bool { return state < 5; }, [](int& state) { ++state; });

        auto const result = std::vector(range.begin(), range.iend());

        CHECK(result == std::vector{0, 1, 2, 3, 4});
    }

    TEST_CASE("simple range: projected")
    {
        auto range = chef::_simple_range(std::pair{0, 5},
            [](auto const& state) -> int const& { return state.first; },
            [](auto state) -> bool { return state.first < state.second; },
            [](auto& state) { ++state.first; });

        auto const result = std::vector(range.begin(), range.iend());

        CHECK(result == std::vector{0, 1, 2, 3, 4});
    }
}
