#include "./re.hpp"

#include <catch2/catch.hpp>

TEST_CASE("re literal concatentation yields a literal")
{
	const chef::re cat = chef::re("Hello") << chef::re(", ") << chef::re("World!");
	REQUIRE(std::holds_alternative<chef::re_lit>(cat.value));
	CHECK(std::get<chef::re_lit>(cat.value).value == "Hello, World!");
}

TEST_CASE("re cat concatentation yields a single cat")
{
	const chef::re cat = chef::re(chef::re_char_class{})
		<< chef::re(chef::re_char_class{}) << chef::re(chef::re_char_class{});
	REQUIRE(std::holds_alternative<chef::re_cat>(cat.value));
	REQUIRE(std::get<chef::re_cat>(cat.value).pieces.size() == 3);

	REQUIRE(std::get<chef::re_cat>(cat.value).pieces[0]);
	REQUIRE(std::get<chef::re_cat>(cat.value).pieces[1]);
	REQUIRE(std::get<chef::re_cat>(cat.value).pieces[2]);

	CHECK(std::holds_alternative<chef::re_char_class>(
		std::get<chef::re_cat>(cat.value).pieces[0]->value));
	CHECK(std::holds_alternative<chef::re_char_class>(
		std::get<chef::re_cat>(cat.value).pieces[1]->value));
	CHECK(std::holds_alternative<chef::re_char_class>(
		std::get<chef::re_cat>(cat.value).pieces[2]->value));
}

TEST_CASE("re union unioning yields a single union")
{
	const chef::re union_ = chef::re(chef::re_char_class{}) | chef::re(chef::re_char_class{})
		| chef::re(chef::re_char_class{});
	REQUIRE(std::holds_alternative<chef::re_union>(union_.value));
	REQUIRE(std::get<chef::re_union>(union_.value).pieces.size() == 3);

	REQUIRE(std::get<chef::re_union>(union_.value).pieces[0]);
	REQUIRE(std::get<chef::re_union>(union_.value).pieces[1]);
	REQUIRE(std::get<chef::re_union>(union_.value).pieces[2]);

	CHECK(std::holds_alternative<chef::re_char_class>(
		std::get<chef::re_union>(union_.value).pieces[0]->value));
	CHECK(std::holds_alternative<chef::re_char_class>(
		std::get<chef::re_union>(union_.value).pieces[1]->value));
	CHECK(std::holds_alternative<chef::re_char_class>(
		std::get<chef::re_union>(union_.value).pieces[2]->value));
}

TEST_CASE("re star starring yields a single star")
{
	const chef::re star = **chef::re(chef::re_char_class{});
	REQUIRE(std::holds_alternative<chef::re_star>(star.value));
	REQUIRE(std::get<chef::re_star>(star.value).value);
	CHECK(std::holds_alternative<chef::re_char_class>(
		std::get<chef::re_star>(star.value).value->value));
}
