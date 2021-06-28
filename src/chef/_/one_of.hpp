#pragma once

#include <concepts>
#include <type_traits>

namespace chef::detail {
	template <typename T, typename... Ts>
	concept DecaysOneOf = (std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<Ts>> || ...);
}
