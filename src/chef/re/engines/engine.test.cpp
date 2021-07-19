#include <chef/re/engines/backtracking.hpp>
#include <chef/re/engines/derivative.hpp>
#include <chef/re/engines/dfa.hpp>

#include <catch2/catch.hpp>

namespace {
	template <typename...>
	struct type_list { };
}

using engines = type_list<chef::re_derivative_engine, chef::re_dfa_engine>;

TEMPLATE_LIST_TEST_CASE("Simple match", "", engines)
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

TEMPLATE_LIST_TEST_CASE("More difficult match", "", engines)
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
TEMPLATE_LIST_TEST_CASE("Epsilon transitions", "", engines)
{
	using Engine = TestType;
	chef::re const re = *(chef::re("a") << *(chef::re("ab"))) | *chef::re("b");

	CHECK(Engine::matches(re, ""));
	CHECK(Engine::matches(re, "a"));
	CHECK(Engine::matches(re, "aab"));
	// Not allowed to switch between options:
	CHECK_FALSE(Engine::matches(re, "aabba"));
}
