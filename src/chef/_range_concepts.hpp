#pragma once

#include <cstddef>
#include <iterator>

#include <chef/_std_concepts.hpp>

namespace chef::_concepts {
    // clang-format off
    template <typename Range>
    concept Sizeable = requires (Range const& range) {
        { std::size(range) } -> _std::convertible_to<std::size_t>;
    };
    // clang-format on
}
