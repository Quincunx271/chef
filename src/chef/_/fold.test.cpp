#include "./fold.hpp"

#include <functional>
#include <type_traits>
#include <vector>

#include <catch2/catch.hpp>

TEST_CASE("Fold works")
{
    int sum = chef::_fold(std::vector{1, 2, 3, 4, 5}, 0, std::plus());
    CHECK(sum == 1 + 2 + 3 + 4 + 5);
}

TEST_CASE("Fold deduces")
{
    auto const vec = std::vector{1, 2, 3, 4, 5};
    bool passed = false;
    
    [[maybe_unused]] int sum = chef::_fold(vec, chef::_deduce([&]<typename T>(chef::_tag_t<T>) {
        passed = std::is_same_v<T, int>;
        return 0;
    }),
        std::plus());
    CHECK(passed);
}
