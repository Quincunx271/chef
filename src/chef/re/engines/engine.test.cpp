#include <chef/re/engines/backtracking.hpp>
#include <chef/re/engines/derivative.hpp>

#include <catch2/catch.hpp>

#define ENGINES chef::re_derivative_engine

TEMPLATE_TEST_CASE("Simple match", "", ENGINES)
{
	using Engine = TestType;
	chef::re const re
		= *(chef::re("Hello, World!") | *(chef::re("a") << (chef::re("b") | chef::re("c"))));

	CHECK(Engine::matches(re, ""));
	CHECK(Engine::matches(re, "Hello, World!"));
	CHECK(Engine::matches(re, "ab"));
	CHECK(Engine::matches(re, "abac"));
	CHECK(Engine::matches(re, "Hello, World!Hello, World!"));
	CHECK(Engine::matches(re, "Hello, World!abababacacHello, World!"));

	CHECK_FALSE(Engine::matches(re, "a"));
	CHECK_FALSE(Engine::matches(re, "Doesn't match"));
}

TEMPLATE_TEST_CASE("More difficult match", "", ENGINES)
{
	using Engine = TestType;
	chef::re const re = (chef::re("ab") | chef::re("a")) << chef::re("baby");

	CHECK(Engine::matches(re, "ababy"));
	CHECK(Engine::matches(re, "abbaby"));
}
