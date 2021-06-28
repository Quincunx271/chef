#pragma once

#include <string_view>

#include <chef/re/re.hpp>

namespace chef {
	struct re_derivative_engine {
		static bool matches(chef::re const& re, std::string_view str);
	};

	auto derivative(chef::re const& re, char c) -> chef::re;
	auto derivative(chef::re const& re, std::string_view str) -> chef::re;
}
