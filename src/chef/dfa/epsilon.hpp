#pragma once

#include <functional>
#include <utility>

namespace chef {
    struct epsilon_t {};

    constexpr inline epsilon_t epsilon;

    [[nodiscard]] constexpr bool operator==(epsilon_t, epsilon_t) { return true; }

    [[nodiscard]] constexpr bool operator!=(epsilon_t, epsilon_t) { return false; }
}

namespace std {
    template <>
    struct hash<chef::epsilon_t> {
        auto operator()(chef::epsilon_t) const { return hash<int>()(0); }
    };
}
