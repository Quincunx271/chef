#pragma once

#include <type_traits>

namespace chef::_std {
    template <typename From, typename To>
    concept convertible_to = std::is_convertible_v<From, To>;
}
