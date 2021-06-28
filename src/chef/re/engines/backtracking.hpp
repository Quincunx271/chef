#pragma once

#include <string_view>

#include <chef/re/re.hpp>

namespace chef {
	struct re_backtracking_engine {
		static bool matches(chef::re const& re, std::string_view str);
	};
}
