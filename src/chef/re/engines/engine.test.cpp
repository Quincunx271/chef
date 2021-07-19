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

// Test case that makes | require epsilon transitions. ((a(ab)*)*|b*)
// The issue (with cat too): if the initial state has incoming edges, there can be a problem.
// This can only happen with a RE if we had a kleene star. Or maybe there is an issue at all if
// the initial state is an accept state.
TEMPLATE_TEST_CASE("Epsilon transitions", "", ENGINES)
{
	using Engine = TestType;
	chef::re const re = *(chef::re("a") << *(chef::re("ab"))) | *chef::re("b");

	CHECK(Engine::matches(re, ""));
	CHECK(Engine::matches(re, "a"));
	CHECK(Engine::matches(re, "aab"));
	// Not allowed to switch between options:
	CHECK_FALSE(Engine::matches(re, "aabba"));
}
