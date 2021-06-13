#include "./set.hpp"

#include <catch2/catch.hpp>

TEST_CASE("set intersect works")
{
	CHECK(chef::detail::intersect(std::unordered_set{1, 2, 3}, std::unordered_set{2, 3, 4})
		== std::unordered_set{2, 3});
	CHECK(chef::detail::intersect(std::unordered_set{2, 3, 4}, std::unordered_set{1, 2, 3})
		== std::unordered_set{2, 3});

	CHECK(chef::detail::intersect(std::unordered_set<int>{}, std::unordered_set{2, 3, 4})
		== std::unordered_set<int>{});
	CHECK(chef::detail::intersect(std::unordered_set{2, 3, 4}, std::unordered_set<int>{})
		== std::unordered_set<int>{});

	CHECK(chef::detail::intersect(std::unordered_set{1, 2, 3}, std::unordered_set{4, 5, 6})
		== std::unordered_set<int>{});

	CHECK(chef::detail::intersect(std::unordered_set{1, 2, 3}, std::unordered_set{1, 2, 3})
		== std::unordered_set{1, 2, 3});

	CHECK(chef::detail::intersect(std::unordered_set{1, 2, 3, 4}, std::unordered_set{1, 2, 3})
		== std::unordered_set{1, 2, 3});
	CHECK(chef::detail::intersect(std::unordered_set{1, 2, 3}, std::unordered_set{1, 2, 3, 4})
		== std::unordered_set{1, 2, 3});
}

TEST_CASE("set difference works")
{
	CHECK(chef::detail::difference(std::unordered_set{1, 2, 3}, std::unordered_set{2, 3, 4})
		== std::unordered_set{1});
	CHECK(chef::detail::difference(std::unordered_set{2, 3, 4}, std::unordered_set{1, 2, 3})
		== std::unordered_set{4});

	CHECK(chef::detail::difference(std::unordered_set<int>{}, std::unordered_set{2, 3, 4})
		== std::unordered_set<int>{});
	CHECK(chef::detail::difference(std::unordered_set{2, 3, 4}, std::unordered_set<int>{})
		== std::unordered_set{2, 3, 4});

	CHECK(chef::detail::difference(std::unordered_set{1, 2, 3}, std::unordered_set{4, 5, 6})
		== std::unordered_set{1, 2, 3});

	CHECK(chef::detail::difference(std::unordered_set{1, 2, 3}, std::unordered_set{1, 2, 3})
		== std::unordered_set<int>{});

	CHECK(chef::detail::difference(std::unordered_set{1, 2, 3, 4}, std::unordered_set{1, 2, 3})
		== std::unordered_set{4});
	CHECK(chef::detail::difference(std::unordered_set{1, 2, 3}, std::unordered_set{1, 2, 3, 4})
		== std::unordered_set<int>{});
}
