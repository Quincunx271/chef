#pragma once

#include <type_traits>

#include <chef/_/fwd.hpp>

namespace chef {
    template <typename... Fs>
    struct _overload_t : Fs... {
        using Fs::operator()...;
    };

    constexpr auto _overload = [](auto&&... fs) {
        return _overload_t<std::remove_cvref_t<decltype(fs)>...>{CHEF_I_FWD(fs)...};
    };
}
