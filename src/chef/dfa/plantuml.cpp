#include "./plantuml.hpp"

#include <ostream>

#include <chef/_/ranges.hpp>
#include <chef/dfa/dfa.hpp>
#include <chef/dfa/nfa.hpp>

namespace chef {
	template <>
	auto to_plantuml<chef::dfa>::do_write(std::ostream& out, to_plantuml<chef::dfa> const& value)
		-> std::ostream&
	{
		out << "@startuml\n";

		auto const& dfa = *value.fa;

		out << "[*] --> " << 0 << '\n';
		for (auto state : dfa.states()) {
			out << std::uint64_t(state) << ":";
			if (auto it = value.labels.find(state); it != value.labels.end()) {
				out << it->second;
			}
			out << '\n';

			for (auto symbol : dfa.symbols()) {
				auto next = dfa.process(state, symbol);
				out << std::uint64_t(state) << " --> " << std::uint64_t(next) << " : "
					<< std::uint64_t(symbol) << '\n';
			}
		}

		return out << "@enduml";
	}

	template <>
	auto to_plantuml<chef::nfa>::do_write(std::ostream& out, to_plantuml<chef::nfa> const& value)
		-> std::ostream&
	{
		out << "@startuml\n";

		auto const& nfa = *value.fa;

		out << "[*] --> " << 0 << '\n';
		for (auto state : nfa.states()) {
			out << std::uint64_t(state) << ":";
			if (auto it = value.labels.find(state); it != value.labels.end()) {
				out << it->second;
			}
			out << '\n';

			for (auto symbol : nfa.symbols()) {
				auto next = nfa.process(state, symbol);
				for (auto next_state : next) {
					out << std::uint64_t(state) << " --> " << std::uint64_t(next_state) << " : "
						<< std::uint64_t(symbol) << '\n';
				}
			}
		}

		return out << "@enduml";
	}
}
