#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <type_traits>

#include <chef/_std_concepts.hpp>

namespace chef {
    namespace _concepts {
        template <typename Arr, typename ArrTo>
        concept SpanConvertible
            = _std::convertible_to<decltype(std::data(std::declval<Arr>())) (*)[], ArrTo>;
    }

    template <typename T>
    class _span {
    public: // Typedefs:
        using element_type = T;
        using value_type = std::remove_cv_t<T>;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using pointer = element_type*;
        using const_pointer = element_type const*;
        using reference = element_type&;
        using const_reference = element_type const&;

        using iterator = element_type*;
        using const_iterator = element_type const*;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    public: // Constructors:
        constexpr _span() = default;

        explicit constexpr _span(pointer first, size_type size)
            : begin_(first)
            , end_(first + size)
        {}

        template <_concepts::SpanConvertible<element_type> Arr>
        constexpr _span(Arr& arr)
            : _span(std::data(arr), std::size(arr))
        {}

    public: // Accessors
        constexpr auto size() const -> size_type { return end_ - begin_; }

        constexpr auto data() const -> pointer { return begin_; }

        constexpr auto operator[](size_type const index) const -> reference
        {
            assert(index < size());
            return begin_[index];
        }

    public: // Iterators:
        constexpr auto begin() const noexcept -> iterator { return begin_; }

        constexpr auto cbegin() const noexcept -> const_iterator { return begin(); }

        constexpr auto rbegin() const noexcept -> reverse_iterator
        {
            return std::reverse_iterator(begin());
        }

        constexpr auto crbegin() const noexcept -> const_reverse_iterator
        {
            return std::reverse_iterator(cbegin());
        }

        constexpr auto end() const noexcept -> iterator { return end_; }

        constexpr auto cend() const noexcept -> const_iterator { return end_; }

        constexpr auto rend() const noexcept -> reverse_iterator
        {
            return std::reverse_iterator(end());
        }

        constexpr auto crend() const noexcept -> const_reverse_iterator
        {
            return std::reverse_iterator(end());
        }

    private:
        element_type* begin_ = nullptr;
        element_type* end_ = nullptr;
    };
}
