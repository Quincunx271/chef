#include "./reserve_as.hpp"

#include <vector>

#include <catch2/catch.hpp>

TEST_CASE("reserve_as works")
{
    auto vec = chef::_reserve_as(std::vector<int>(), 123);
    CHECK(vec.capacity() == 123);
}

TEST_CASE("reserve_as deduced works")
{
    std::vector<int> vec = chef::_reserve_as(123);
    CHECK(vec.capacity() == 123);
}
