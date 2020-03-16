#pragma once

#include <utility>

#include <chef/_/simple_range.hpp>

namespace chef {
    constexpr auto _transform = [](auto&& range, auto&& f) {
        return chef::_simple_range(std::pair(range.begin(), range.end()),
            [f = std::forward<decltype(f)>(f)](auto&& state) -> decltype(auto) { //
                return f(*state.first);
            },
            [](auto&& state) -> bool { return state.first != state.second; },
            [](auto& state) { ++state.first; });
    };
}
