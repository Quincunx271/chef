#include "./dfa.hpp"

#include <chef/dfa/convert.hpp>
#include <chef/dfa/minimize.hpp>
#include <chef/re/to_nfa.hpp>

using chef::detail::overload;
namespace ranges = std::ranges;
namespace views = ranges::views;
using ranges::begin;
using ranges::end;

namespace chef {
	bool re_dfa_engine::matches(chef::re const& re, std::string_view str)
	{
		auto nfa_result = chef::to_nfa(re);

		std::vector<std::unordered_set<state_type>> categories;
		categories.push_back(CHEF_MOVE(nfa_result.accepts));

		auto [dfa, dfa_categories] = chef::to_dfa(nfa_result.nfa, categories);
		auto [min_dfa, min_dfa_categories] = chef::minimize(dfa, dfa_categories);

		chef::state_type cur = 0;
		for (const char c : str) {
			auto it = nfa_result.symbol_map.find(c);
			if (it == nfa_result.symbol_map.end()) return false;
			cur = min_dfa.process(cur, it->second);
		}

		return min_dfa_categories[0].contains(cur);
	}
}
