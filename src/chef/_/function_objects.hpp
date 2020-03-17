#pragma once

#include <algorithm>
#include <functional>
#include <tuple>

#include <chef/_/fwd.hpp>

namespace chef {
    constexpr auto _max
        = [](auto&&... args) -> decltype(auto) { return std::max(CHEF_I_FWD(args)...); };

    constexpr auto _min
        = [](auto&&... args) -> decltype(auto) { return std::min(CHEF_I_FWD(args)...); };

    template <auto Index>
    constexpr auto _get_index
        = [](auto&& tuple) -> decltype(auto) { return std::get<Index>(CHEF_I_FWD(tuple)); };

    constexpr auto _identity = [](auto&& val) -> decltype(auto) { return CHEF_I_FWD(val); };

    template <typename F>
    class _by_value {
        [[no_unique_address]] F fn;

    public:
        template <typename G>
        explicit constexpr _by_value(G&& g)
            : fn(CHEF_I_FWD(g))
        {}

        template <typename... Args>
        constexpr auto operator()(Args&&... args) const&
        {
            return fn(CHEF_I_FWD(args)...);
        }

        template <typename... Args>
        constexpr auto operator()(Args&&... args) &
        {
            return fn(CHEF_I_FWD(args)...);
        }

        template <typename... Args>
        constexpr auto operator()(Args&&... args) const&&
        {
            return CHEF_I_MOVE(fn)(CHEF_I_FWD(args)...);
        }

        template <typename... Args>
        constexpr auto operator()(Args&&... args) &&
        {
            return CHEF_I_MOVE(fn)(CHEF_I_FWD(args)...);
        }
    };

    template <typename F>
    explicit _by_value(F)->_by_value<F>;
}
