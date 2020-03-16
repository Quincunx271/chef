#pragma once

#include <chef/_/simple_range.hpp>
#include <chef/_/std_concepts.hpp>

namespace chef {
    constexpr auto _bit_indices = []<_std::unsigned_integral T>(T bits) {
        struct state_t {
            T bits;
            std::uint8_t index;
        } state{
            .bits = bits,
            .index = 0,
        };

        constexpr auto advance = [](state_t& state) {
            do {
                state.bits = static_cast<T>(state.bits >> 1);
                ++state.index;
            } while (state.bits && !(state.bits & 0x1));
        };

        if (!(state.bits & 0x1)) advance(state);

        return _simple_range(state, //
            [](auto&& state) -> std::uint8_t const& { return state.index; },
            [](auto&& state) -> bool { return state.bits; }, //
            advance);
    };
}
