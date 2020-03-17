#pragma once

#include <iterator>

#include <chef/_/function_objects.hpp>
#include <chef/_/fwd.hpp>
#include <chef/_/simple_range.hpp>

namespace chef {
    constexpr auto _filter_advance_until = [](auto& iter, auto const& end, auto&& pred) {
        while (iter != end && !pred(*iter))
            ++iter;
    };

    constexpr auto _filter = [](auto&& range, auto&& pred) {
        using std::begin;
        using std::end;

        auto first = begin(range);
        auto last = end(range);
        _filter_advance_until(first, last, pred);

        return chef::_simple_range(std::pair(first, last),
            [](auto&& state) -> decltype(auto) { return *state.first; },
            [](auto&& state) -> bool { return state.first != state.second; },
            [pred = CHEF_I_FWD(pred)](auto& state) {
                ++state.first;
                _filter_advance_until(state.first, state.second, pred);
            });
    };
}
