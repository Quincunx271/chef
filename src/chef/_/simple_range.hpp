#pragma once

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

namespace chef {
    template <typename T, typename ReadF, typename IsValidF, typename NextF>
    class _simple_range {
        T state_;
        [[no_unique_address]] ReadF read_;
        [[no_unique_address]] IsValidF is_valid_;
        [[no_unique_address]] NextF next_;

    public:
        explicit constexpr _simple_range(T state, ReadF read, IsValidF is_valid, NextF next)
            : state_(std::move(state))
            , read_(std::move(read))
            , is_valid_(std::move(is_valid))
            , next_(std::move(next))
        {}

        struct sentinel {};

        class iterator {
            friend _simple_range;

        public:
            using value_type
                = std::remove_cvref_t<decltype(std::declval<ReadF&>()(std::declval<T const&>()))>;
            using reference = value_type const&;
            using pointer = value_type const*;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::input_iterator_tag;

        private:
            _simple_range* range_ = nullptr;

            explicit constexpr iterator(_simple_range* range)
                : range_(range)
            {}

        public:
            iterator() = default;

            auto operator*() const -> reference { return range_->read_(range_->state_); }
            auto operator-> () const -> pointer { return &**this; }

            auto operator++() -> iterator&
            {
                range_->next_(range_->state_);
                return *this;
            }

            bool operator==(sentinel) const noexcept { return !range_->is_valid_(range_->state_); }

            friend bool operator==(sentinel, iterator const rhs) noexcept
            {
                return rhs == sentinel{};
            }

            friend bool operator!=(iterator const lhs, sentinel) noexcept
            {
                return !(lhs == sentinel{});
            }

            friend bool operator!=(sentinel, iterator const rhs) noexcept
            {
                return rhs != sentinel{};
            }

            friend bool operator==(iterator const lhs, iterator const rhs) noexcept
            {
                if (bool(lhs.range_) == bool(rhs.range_)) return lhs.range_ == rhs.range_;
                if (lhs.range_) return lhs == sentinel{};
                if (rhs.range_) return sentinel{} == rhs;
                return true;
            }

            friend bool operator!=(iterator const lhs, iterator const rhs) noexcept
            {
                return !(lhs == rhs);
            }
        };

        auto begin() -> iterator { return iterator(this); }
        auto end() -> sentinel { return {}; }

        auto iend() -> iterator { return iterator(nullptr); }
    };
}
