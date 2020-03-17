#pragma once

#include <chef/_/deduce.hpp>
#include <chef/_/overload.hpp>

namespace chef {
    constexpr auto _reserve_as = chef::_overload(
        [](auto container, auto const amount) {
            container.reserve(amount);
            return container;
        },
        [](auto const amount) {
            return chef::_deduce([amount]<typename T>(chef::_tag_t<T>) {
                T container;
                container.reserve(amount);
                return container;
            });
        });
}
