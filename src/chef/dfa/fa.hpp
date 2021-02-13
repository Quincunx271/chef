#pragma once

#include <cstdint>

namespace chef {
	using state_type = std::uint32_t;
	using symbol_type = std::uint8_t;

	struct fa_edge {
		state_type from;
		state_type to;
		symbol_type on;
	};
}
