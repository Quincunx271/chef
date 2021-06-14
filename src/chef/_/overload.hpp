#pragma once

namespace chef::detail {
	template <typename... Ls>
	struct overload : Ls... {
		using Ls::operator()...;
	};

	template <typename... Ls>
	overload(Ls...) -> overload<Ls...>;
}
