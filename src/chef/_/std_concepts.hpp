#pragma once

#include <type_traits>

namespace chef::_std {
    template <typename From, typename To>
    concept convertible_to = std::is_convertible_v<From, To>;

    template <typename T>
    concept integral = std::is_integral_v<T>;

    // clang-format off
    template <typename T>
    concept signed_integral = _std::integral<T> && std::is_signed_v<T>;
    // clang-format on

    template <typename T>
    concept unsigned_integral = _std::integral<T> && !_std::signed_integral<T>;
}
