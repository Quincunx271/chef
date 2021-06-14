#include "./value_ptr.hpp"

#include <catch2/catch.hpp>

TEST_CASE("It works well enough")
{
	const chef::detail::value_ptr<int> a(std::make_unique<int>(42));
	chef::detail::value_ptr<int> b(std::make_unique<int>(42));
	const chef::detail::value_ptr<int> c(std::make_unique<int>(-42));
	const chef::detail::value_ptr<int> d = nullptr;

	CHECK(a == b);
	CHECK(a == a);
	CHECK(a != c);
	CHECK(a != d);
	CHECK(a != nullptr);
	CHECK(b != c);
	CHECK(c != d);
	CHECK(d == nullptr);
	CHECK(d < a);
	CHECK(a > nullptr);
	CHECK(c < a);
}
