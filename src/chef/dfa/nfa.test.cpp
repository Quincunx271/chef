#include "./nfa.hpp"

#include <sstream>

#include <catch2/catch.hpp>

TEST_CASE("Simple stuff")
{
    chef::nfa nfa{
        {"_", 'a', "lowercase"},
        {"_", 'A', "_"},
        {"_", chef::epsilon, "asdf"},
        {"lowercase", 'a', "lowercase"},
        {"lowercase", 'A', "_"},
    };

    std::ostringstream out;
    out << nfa;
    // CHECK(out.str() == "");
}
