#pragma once

#include <type_traits>

#include <chef/_/simple_range.hpp>

namespace chef {
    constexpr auto _irange = [](auto low, auto high) {
        using the_type_t = std::common_type_t<decltype(low), decltype(high)>;

        return chef::_simple_range(std::pair(the_type_t(low), the_type_t(high)),
            [](auto&& state) -> the_type_t const& { return state.first; },
            [](auto&& state) -> bool { return state.first != state.second; },
            [](auto& state) { ++state.first; });
    };
}
