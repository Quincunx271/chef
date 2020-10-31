#pragma once

#include <iterator>

#include <chef/_/fwd.hpp>

namespace chef {
    template <typename T>
    struct _initlist {
        template <std::size_t N>
        constexpr _initlist(T(&&init)[N])
            : data {CHEF_I_MOVE(init)}
            , size {N}
        { }

        template <typename Range>
        constexpr operator Range() &&
        {
            return Range(std::move_iterator(data + 0), std::move_iterator(data + size));
        }

    private:
        T(&&data)[];
        std::size_t size;
    };
}
