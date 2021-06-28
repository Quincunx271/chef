#pragma once

#include <compare>
#include <cstddef>
#include <memory>
#include <tuple>
#include <utility>

namespace chef::detail {
	template <typename T>
	class value_ptr {
	private:
		std::unique_ptr<T> value;

	public:
		value_ptr(std::nullptr_t = nullptr)
			: value(nullptr)
		{ }

		explicit value_ptr(std::unique_ptr<T> value)
			: value(std::move(value))
		{ }

		value_ptr(value_ptr const& rhs)
			: value(rhs ? std::make_unique<T>(*rhs) : nullptr)
		{ }

		value_ptr(value_ptr&& rhs) noexcept
			: value(std::move(rhs.value))
		{ }

		value_ptr& operator=(value_ptr const& rhs)
		{
			value = rhs ? std::make_unique<T>(*rhs) : nullptr;
			return *this;
		}

		value_ptr& operator=(value_ptr&& rhs) noexcept
		{
			value = std::move(rhs.value);
			return *this;
		}

		T& operator*()
		{
			return *value;
		}

		T const& operator*() const
		{
			return *value;
		}

		T* operator->()
		{
			return value.get();
		}

		T const* operator->() const
		{
			return value.get();
		}

		T* get()
		{
			return value.get();
		}

		T const* get() const
		{
			return value.get();
		}

		explicit operator bool() const
		{
			return static_cast<bool>(value);
		}

		bool operator==(value_ptr const& rhs) const
		{
			if (value == rhs.value) return true;
			if (value && rhs.value) {
				return *value == *rhs;
			}
			return false;
		}

		bool operator==(std::nullptr_t) const
		{
			return value == nullptr;
		}

		template <typename U>
			requires std::three_way_comparable_with<T, U>
		auto operator<=>(value_ptr<U> const& rhs) const -> std::compare_three_way_result_t<T, U>
		{
			return std::forward_as_tuple(static_cast<bool>(*this), **this)
				<=> std::forward_as_tuple(static_cast<bool>(rhs), *rhs);
		}

		template <typename U = T>
			requires std::three_way_comparable_with<T, U>
		auto operator<=>(std::nullptr_t) const -> std::compare_three_way_result_t<T, U>
		{
			return (*this <=> value_ptr(nullptr));
		}
	};
}
