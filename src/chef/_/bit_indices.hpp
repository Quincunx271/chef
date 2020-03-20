#pragma once

#include <bitset>
#include <climits>
#include <cstddef>

#include <chef/_/lambda.hpp>
#include <chef/_/overload.hpp>
#include <chef/_/simple_range.hpp>
#include <chef/_/std_concepts.hpp>

namespace chef {
    template <std::size_t N>
    struct _bit_indices_state_t {
        std::bitset<N> bits;
        std::size_t index = 0;

        constexpr bool has_remaining() const { return bits.any(); }

        constexpr void advance_to_next()
        {
            if (!has_remaining()) return;

            while (!bits.test(0))
                increment();
        }

        constexpr void advance()
        {
            increment();
            advance_to_next();
        }

        constexpr void increment()
        {
            ++index;
            bits >>= 1;
        }
    };

    constexpr auto _bit_indices_impl = []<std::size_t N>(std::bitset<N> bs) {
        using state_t = _bit_indices_state_t<N>;

        auto state = state_t{bs};
        state.advance_to_next();

        return _simple_range(state, //
                                    // get():
            chef::_mem_data<&state_t::index>, //
            // has_next():
            chef::_mem_fn<&state_t::has_remaining>, //
            // next():
            chef::_mem_fn<&state_t::advance>);
    };

    constexpr auto _bit_indices = chef::_overload(
        []<_std::unsigned_integral T>(
            T bits) { return _bit_indices_impl(std::bitset<sizeof(T) * CHAR_BIT>(bits)); },
        _bit_indices_impl);
}
