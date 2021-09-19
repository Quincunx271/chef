#pragma once

#include <ranges>

namespace chef {
	template <typename R, typename T>
	concept RangeOf = std::ranges::range<R> && std::same_as<T, std::ranges::range_reference_t<R>>;

	template <typename R, typename T>
	concept InputRangeOf = RangeOf<R, T> && std::ranges::input_range<R>;

	template <typename R, typename T>
	concept ForwardRangeOf = InputRangeOf<R, T> && std::ranges::forward_range<R>;
}
