#pragma once

#include <chef/_/fwd.hpp>

#define CHEF_I_MEMFN(memfn)                                                                        \
    [](auto&& obj, auto&&... args) noexcept(noexcept(CHEF_I_FWD(obj) memfn(CHEF_I_FWD(args)...)))  \
        ->decltype(CHEF_I_FWD(obj) memfn(CHEF_I_FWD(args)...))                                     \
    {                                                                                              \
        return CHEF_I_FWD(obj) memfn(CHEF_I_FWD(args)...);                                         \
    }

#define CHEF_I_MEMFN_MUT(memfn)                                                                    \
    [](auto&& obj, auto&&... args) noexcept(noexcept(CHEF_I_FWD(obj) memfn(CHEF_I_FWD(args)...)))  \
        ->decltype(CHEF_I_FWD(obj))                                                                \
    {                                                                                              \
        CHEF_I_FWD(obj) memfn(CHEF_I_FWD(args)...);                                                \
        return CHEF_I_FWD(obj);                                                                    \
    }

#define CHEF_I_L(...)                                                                              \
    ([[maybe_unused]] auto&& it) noexcept(noexcept(__VA_ARGS__))->decltype(__VA_ARGS__)            \
    {                                                                                              \
        return (__VA_ARGS__);                                                                      \
    }

namespace chef {
    template <auto V>
    constexpr auto _mem_data = [](auto&& obj) noexcept -> decltype(auto)
    {
        return (obj.*V);
    };

    template <auto F>
    constexpr auto _mem_fn = [](auto&& obj, auto&&... args) noexcept(
        noexcept((obj.*F)(CHEF_I_FWD(args)...))) -> decltype((obj.*F)(CHEF_I_FWD(args)...))
    {
        return (obj.*F)(CHEF_I_FWD(args)...);
    };
}
