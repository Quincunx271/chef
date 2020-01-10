#pragma once

#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <numeric>
#include <type_traits>

namespace chef {
    // row-major multidimensional array view
    template <typename Container, int DimensionCount>
    class _mdview {
    public:
        using underlying_type = Container;
        static constexpr auto num_dimensions = DimensionCount;

        template <typename... Dims>
        explicit constexpr _mdview(Container& container,
            Dims const... dims) noexcept //
            requires(sizeof...(Dims) == num_dimensions
                && (std::convertible_to<std::size_t, Dims const> && ...))
            : _mdview(container, {std::size_t(dims)...})
        {}

        explicit constexpr _mdview(
            Container& container, std::array<std::size_t, num_dimensions> const dimensions) noexcept
            : container_(&container)
            , dimensions_(dimensions)
        {
            // if constexpr (std::ranges::sized_range<Container>) {
            //      assert(std::accumulate(dimensions_.begin(), dimensions_.end(), 1,
            //          std::multiplies()) == std::ranges::size(container));
            // }
        }

        constexpr auto dimensions() const noexcept -> std::array<std::size_t, num_dimensions>
        {
            return dimensions_;
        }

        constexpr auto container() const -> Container* { return container_; }

        constexpr auto operator[](std::array<std::size_t, num_dimensions> const indices) const
            -> decltype(auto)
        {
            assert(in_bounds(indices));
            return (*container_)[compute_index(indices)];
        }

        template <typename... Dims>
        constexpr auto at(Dims const... indices) const noexcept
            -> decltype(auto) requires(sizeof...(Dims) == num_dimensions)
        {
            return at({std::size_t(indices)...});
        }

        constexpr auto at(std::array<std::size_t, num_dimensions> const indices) const
            -> decltype(auto)
        {
            if (!in_bounds(indices)) throw std::out_of_range("at least one index is out of range");
            return (*this)[indices];
        }

        template <typename... Dims>
        constexpr auto compute_index(Dims const... indices) const noexcept -> std::size_t
            requires(sizeof...(Dims) == num_dimensions)
        {
            return compute_index({std::size_t(indices)...});
        }

        constexpr auto compute_index(std::array<std::size_t, num_dimensions> const indices) const
            noexcept -> std::size_t
        {
            using std::begin;
            using std::cbegin;
            using std::cend;

            assert(in_bounds(indices));

            std::array<std::size_t, num_dimensions> acc_dimensions;
            acc_dimensions[0] = 1;

            std::partial_sum(cbegin(dimensions_), cend(dimensions_) - 1, begin(acc_dimensions) + 1,
                std::multiplies());

            auto const index = std::transform_reduce(cbegin(acc_dimensions), cend(acc_dimensions),
                cbegin(indices), std::size_t(0), std::plus(), std::multiplies());

            assert(index < compute_size(*container_));

            return index;
        }

    private:
        Container* container_;
        std::array<std::size_t, num_dimensions> dimensions_;

        constexpr bool in_bounds(std::array<std::size_t, num_dimensions> indices) const
        {
            using std::cbegin;
            using std::cend;

            // Every index is less than the corresponding dimension size.
            return std::transform_reduce(cbegin(indices), cend(indices), cbegin(dimensions_), true,
                std::logical_and(), std::less());
        }

        static constexpr auto compute_size(Container const& container) -> std::size_t
            requires(requires {
                // clang-format off
                { std::size(container) } -> std::convertible_to<std::size_t>;
                // clang-format on
            })
        {
            return size(container);
        }

        static constexpr auto compute_size(Container const&) -> std::size_t
        {
            // max size:
            return std::size_t(-1);
        }
    };

    template <int Dims>
    constexpr auto _make_mdview = [](auto& container, auto const... dims) {
        return _mdview<std::remove_reference_t<decltype(container)>, Dims>(container, dims...);
    };
}
