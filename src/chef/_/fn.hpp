#pragma once

#include <functional>

namespace chef::detail {
	constexpr auto equal_to_dangle = [](const auto& x) {
		return [&x](const auto& rhs) { return std::ranges::equal_to{}(x, rhs); };
	};
}
