#pragma once

#include <iterator>
#include <numeric>

#include <chef/_/deduce.hpp>
#include <chef/_/fwd.hpp>
#include <chef/_/tag.hpp>

namespace chef {
    constexpr auto _fold = [](auto&& range, auto&& init, auto&& folder) {
        using std::begin;
        using std::end;

        if constexpr (_is_deduce<decltype(init)>) {
            return chef::_deduce(
                [ range = &range, &init, &folder ]<typename T>(chef::_tag_t<T>) mutable {
                    return std::accumulate(begin(*range), end(*range),
                        static_cast<T>(CHEF_I_FWD(init)), CHEF_I_FWD(folder));
                });
        } else {
            return std::accumulate(begin(range), end(range), CHEF_I_FWD(init), CHEF_I_FWD(folder));
        }
    };
}
