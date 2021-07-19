#pragma once

#include <iosfwd>
#include <string>
#include <unordered_map>

#include <chef/_/fwd.hpp>
#include <chef/dfa/fa.hpp>

namespace chef {
	template <typename FA>
	struct to_plantuml {
	private:
		FA const* fa;
		std::unordered_map<chef::state_type, std::string> labels;

	public:
		explicit to_plantuml(
			FA const& fa, std::unordered_map<chef::state_type, std::string> labels = {})
			: fa{&fa}
			, labels(CHEF_MOVE(labels))
		{ }

		friend auto operator<<(std::ostream& out, to_plantuml const& value) -> std::ostream&
		{
			return do_write(out, value);
		}

	private:
		static auto do_write(std::ostream& out, to_plantuml const& value) -> std::ostream&;
	};
}
