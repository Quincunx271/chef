#include "./deduce.hpp"

#include <string>
#include <type_traits>
#include <typeinfo>

#include <catch2/catch.hpp>

TEST_CASE("Can deduce the type")
{
    std::type_info const* type;

    [[maybe_unused]] int result = chef::_deduce([&]<typename T>(chef::_tag_t<T>)->decltype(auto) {
        type = &typeid(T);
        return 0;
    });

    CHECK(*type == typeid(int));
}

TEMPLATE_TEST_CASE("Will deduce", "", int&, int const&, int&&, int const&&)
{
    int x = 42;
    bool passed = false;

    [[maybe_unused]] TestType result = chef::_deduce([&]<typename T>(chef::_tag_t<T>)->decltype(auto) {
        passed = std::is_same_v<T, TestType>;
        return static_cast<TestType>(x);
    });

    CHECK(passed);
}
