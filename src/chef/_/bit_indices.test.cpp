#include "./bit_indices.hpp"

#include <climits>
#include <numeric>
#include <vector>

#include <catch2/catch.hpp>

namespace {
    TEMPLATE_TEST_CASE("bit_indices is an empty range when no bit is set", "", std::uint8_t,
        std::uint16_t, std::uint32_t, std::uint64_t)
    {
        auto indices = chef::_bit_indices(TestType{0ull});
        CHECK(std::distance(indices.begin(), indices.iend()) == 0);
    }

    TEMPLATE_TEST_CASE("bit_indices with every bit set", "", std::uint8_t, std::uint16_t,
        std::uint32_t, std::uint64_t)
    {
        auto indices = chef::_bit_indices(static_cast<TestType>(-1));
        auto const result = std::vector(indices.begin(), indices.iend());

        auto expected = std::vector<std::size_t>(sizeof(TestType) * CHAR_BIT);
        std::iota(expected.begin(), expected.end(), 0);

        CHECK(result == expected);
    }

    TEMPLATE_TEST_CASE("bit_indices with exactly one bit set", "", std::uint8_t, std::uint16_t,
        std::uint32_t, std::uint64_t)
    {
        auto const bit = GENERATE(range<std::uint8_t>(0, sizeof(TestType) * CHAR_BIT));
        auto indices = chef::_bit_indices(static_cast<TestType>(1ull << bit));
        auto const result = std::vector(indices.begin(), indices.iend());

        auto expected = std::vector<std::size_t>{bit};

        CHECK(result == expected);
    }
}
