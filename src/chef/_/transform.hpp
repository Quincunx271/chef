#pragma once

#include <utility>

#include <chef/_/fwd.hpp>
#include <chef/_/simple_range.hpp>

namespace chef {
    constexpr auto _transform = [](auto&& range, auto&& f) {
        return chef::_simple_range(std::pair(range.begin(), range.end()),
            [f = CHEF_I_FWD(f)](auto&& state) -> decltype(auto) { //
                return f(*state.first);
            },
            [](auto&& state) -> bool { return state.first != state.second; },
            [](auto& state) { ++state.first; });
    };
}
