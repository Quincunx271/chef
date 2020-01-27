#pragma once

#include <cstddef>
#include <iterator>
#include <utility>

namespace chef {
    template <typename Container, typename IndexType = std::size_t>
    class _indexed_t {
    public:
        using index_type = IndexType;

    private:
        struct sentinel_t {};

        using c_iterator = decltype(std::begin(std::declval<Container&>()));
        using c_reference_type = decltype(*std::declval<c_iterator const>());

        struct reference_type {
            index_type index;
            c_reference_type value;
        };

    public:
        explicit constexpr _indexed_t(Container& container)
            : first(std::begin(container))
            , last(std::end(container))
        {}

        constexpr auto begin() const { return *this; }

        constexpr auto end() const { return sentinel_t(); }

        constexpr auto operator++()
        {
            ++index;
            ++first;
            return *this;
        }

        constexpr auto operator*() const { return reference_type {index, *first}; }

        constexpr auto operator==(sentinel_t) const { return first == last; }

        constexpr auto operator!=(sentinel_t) const { return first != last; }

    private:
        c_iterator first;
        c_iterator last;
        index_type index = 0;
    };

    template <typename IndexType = std::size_t, typename Container>
    constexpr auto _indexed(Container& container)
    {
        return _indexed_t<Container, IndexType>(container);
    }
}
