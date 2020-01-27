#include <chef/_mdview.hpp>

#include <catch2/catch.hpp>

TEST_CASE("_mdview computes index correctly")
{
    auto const vec = std::vector<int> {
        // clang-format off
        1, 2, 3, 4,
        5, 6, 7, 8,
        // clang-format on
    };

    auto const mdview = chef::_make_mdview<2>(vec, 4, 2);

    CHECK(mdview.compute_index(0, 0) == 0);
    CHECK(mdview.compute_index(1, 0) == 1);
    CHECK(mdview.compute_index(2, 0) == 2);
    CHECK(mdview.compute_index(3, 0) == 3);
    CHECK(mdview.compute_index(0, 1) == 4);
    CHECK(mdview.compute_index(1, 1) == 5);
    CHECK(mdview.compute_index(2, 1) == 6);
    CHECK(mdview.compute_index(3, 1) == 7);
}

TEST_CASE("_mdview accesses correctly")
{
    auto vec = std::vector<int> {
        // clang-format off
        1, 2, 3, 4,
        5, 6, 7, 8,
        // clang-format on
    };

    auto const mdview = chef::_make_mdview<2>(vec, 4, 2);

    CHECK(mdview.at(0, 0) == 1);
    CHECK(mdview.at(1, 0) == 2);
    CHECK(mdview.at(2, 0) == 3);
    CHECK(mdview.at(3, 0) == 4);
    CHECK(mdview.at(0, 1) == 5);
    CHECK(mdview.at(1, 1) == 6);
    CHECK(mdview.at(2, 1) == 7);
    CHECK(mdview.at(3, 1) == 8);

    CHECK(mdview[{0, 0}] == 1);
    CHECK(mdview[{1, 0}] == 2);
    CHECK(mdview[{2, 0}] == 3);
    CHECK(mdview[{3, 0}] == 4);
    CHECK(mdview[{0, 1}] == 5);
    CHECK(mdview[{1, 1}] == 6);
    CHECK(mdview[{2, 1}] == 7);
    CHECK(mdview[{3, 1}] == 8);

    mdview[{2, 1}] = 7;
    CHECK(mdview[{2, 1}] == 7);
    CHECK(vec.at(1 * 4 + 2) == 7);
}

TEST_CASE("_mdview works for 1 dimensional view")
{
    auto const vec = std::vector<int> {0, 1, 2, 3, 4, 5, 6, 7};

    auto const mdview = chef::_make_mdview<1>(vec, vec.size());

    CHECK(mdview.compute_index(0) == 0);
    CHECK(mdview.compute_index(1) == 1);
    CHECK(mdview.compute_index(2) == 2);
    CHECK(mdview.compute_index(3) == 3);
    CHECK(mdview.compute_index(4) == 4);
    CHECK(mdview.compute_index(5) == 5);
    CHECK(mdview.compute_index(6) == 6);
    CHECK(mdview.compute_index(7) == 7);
}

TEST_CASE("_mdview works for 3 dimensional view")
{
    auto const vec = std::vector<int> {
        // clang-format off
        111, 112, 113, 114,
        121, 122, 123, 124,

        211, 212, 213, 214,
        221, 222, 223, 224,

        311, 312, 313, 314,
        321, 322, 323, 324,
        // clang-format on
    };

    auto const mdview = chef::_make_mdview<3>(vec, 4, 2, 3);

    for (std::size_t z = 1; z <= 3; ++z) {
        for (std::size_t y = 1; y <= 2; ++y) {
            for (std::size_t x = 1; x <= 4; ++x) {
                CAPTURE(x, y, z);
                CHECK(mdview[{x - 1, y - 1, z - 1}] == z * 100 + y * 10 + x);
            }
        }
    }
}
