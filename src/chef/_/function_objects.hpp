#pragma once

#include <algorithm>
#include <functional>
#include <tuple>
#include <utility>

namespace chef {
    constexpr auto _max = [](auto&&... args) -> decltype(auto) {
        return std::max(std::forward<decltype(args)>(args)...);
    };

    constexpr auto _min = [](auto&&... args) -> decltype(auto) {
        return std::min(std::forward<decltype(args)>(args)...);
    };

    template <auto Index>
    constexpr auto _get_index = [](auto&& tuple) -> decltype(auto) {
        return std::get<Index>(std::forward<decltype(tuple)>(tuple));
    };

    constexpr auto _identity
        = [](auto&& val) -> decltype(auto) { return std::forward<decltype(val)>(val); };
}
