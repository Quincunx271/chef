#include "./basic.hpp"

#include <chef/_/initlist.hpp>

#include <catch2/catch.hpp>

using namespace std::literals;

TEST_CASE("basic engine - literal")
{
    auto re = chef::_regex::regex {"Hello, World"s};
    CHECK(chef::basic_regex_engine::matches(re, "Hello, World"sv));
    CHECK_FALSE(chef::basic_regex_engine::matches(re, "Hello, World "sv));
    CHECK_FALSE(chef::basic_regex_engine::matches(re, ""sv));
}

TEST_CASE("basic engine - seq")
{
    auto re = chef::_regex::regex {chef::_regex::sequence {chef::_initlist({
        std::make_unique<chef::_regex::regex>(chef::_regex::regex {"Hello"s}),
        std::make_unique<chef::_regex::regex>(chef::_regex::regex {", "s}),
        std::make_unique<chef::_regex::regex>(chef::_regex::regex {"World"s}),
    })}};
    CHECK(chef::basic_regex_engine::matches(re, "Hello, World"sv));
    CHECK_FALSE(chef::basic_regex_engine::matches(re, "Hello, World "sv));
    CHECK_FALSE(chef::basic_regex_engine::matches(re, "Hello"sv));
    CHECK_FALSE(chef::basic_regex_engine::matches(re, ""sv));
}

TEST_CASE("basic engine - alt")
{
    auto re = chef::_regex::regex {chef::_regex::alternative {chef::_initlist({
        std::make_unique<chef::_regex::regex>(chef::_regex::regex {"Hello"s}),
        std::make_unique<chef::_regex::regex>(chef::_regex::regex {", "s}),
        std::make_unique<chef::_regex::regex>(chef::_regex::regex {"World"s}),
    })}};
    CHECK(chef::basic_regex_engine::matches(re, "Hello"sv));
    CHECK(chef::basic_regex_engine::matches(re, ", "sv));
    CHECK(chef::basic_regex_engine::matches(re, "World"sv));
    CHECK_FALSE(chef::basic_regex_engine::matches(re, "Hello "sv));
    CHECK_FALSE(chef::basic_regex_engine::matches(re, ",  "sv));
    CHECK_FALSE(chef::basic_regex_engine::matches(re, "World "sv));
    CHECK_FALSE(chef::basic_regex_engine::matches(re, "Hello, "sv));
    CHECK_FALSE(chef::basic_regex_engine::matches(re, ""sv));
}

TEST_CASE("basic engine - *")
{
    auto re = chef::_regex::regex {chef::_regex::kleene_star {
        std::make_unique<chef::_regex::regex>(chef::_regex::regex {"Hello"s})}};
    CHECK(chef::basic_regex_engine::matches(re, ""sv));
    CHECK(chef::basic_regex_engine::matches(re, "Hello"sv));
    CHECK(chef::basic_regex_engine::matches(re, "HelloHello"sv));
    CHECK(chef::basic_regex_engine::matches(re, "HelloHelloHelloHelloHello"sv));
    CHECK(chef::basic_regex_engine::matches(
        re, "HelloHelloHelloHelloHelloHelloHelloHelloHelloHello"sv));
    CHECK_FALSE(chef::basic_regex_engine::matches(re, " "sv));
    CHECK_FALSE(chef::basic_regex_engine::matches(re, "Hello "sv));
    CHECK_FALSE(chef::basic_regex_engine::matches(re, "HelloHel"sv));
}

TEST_CASE("basic engine - some arbitrary thing")
{
    using namespace chef::_regex;

    // ((A|B*)C)*
    auto const re = regex {kleene_star {
        std::make_unique<regex>(regex {
            sequence {chef::_initlist({
                std::make_unique<regex>(regex {alternative {chef::_initlist({
                    std::make_unique<regex>(regex {"A"s}),
                    std::make_unique<regex>(
                        regex {kleene_star {std::make_unique<regex>(regex {"B"s})}}),
                })}}),
                std::make_unique<regex>(regex {"C"s}),
            })},
        }),
    }};

    CHECK(chef::basic_regex_engine::matches(re, ""sv));
    CHECK(chef::basic_regex_engine::matches(re, "C"sv));
    CHECK(chef::basic_regex_engine::matches(re, "AC"sv));
    CHECK(chef::basic_regex_engine::matches(re, "BC"sv));
    CHECK(chef::basic_regex_engine::matches(re, "BBBC"sv));
    CHECK(chef::basic_regex_engine::matches(re, "BBBCAC"sv));
    CHECK(chef::basic_regex_engine::matches(re, "BBBCBC"sv));
    CHECK(chef::basic_regex_engine::matches(re, "ACBBC"sv));
    CHECK(chef::basic_regex_engine::matches(re, "ACACACBBCACBC"sv));
    CHECK_FALSE(chef::basic_regex_engine::matches(re, " "sv));
    CHECK_FALSE(chef::basic_regex_engine::matches(re, "AC "sv));
    CHECK_FALSE(chef::basic_regex_engine::matches(re, "BC "sv));
    CHECK_FALSE(chef::basic_regex_engine::matches(re, "BBBC "sv));
    CHECK_FALSE(chef::basic_regex_engine::matches(re, "A"sv));
    CHECK_FALSE(chef::basic_regex_engine::matches(re, "B"sv));
}
