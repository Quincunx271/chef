#include "./ll1.hpp"

#include <map>
#include <set>
#include <stdexcept>

#include <chef/cfg/cfg.hpp>

namespace chef {
	namespace {
		auto compute_ll1_table(const cfg& grammar)
		{
			std::map<cfg_var, std::map<cfg_token, cfg_seq>> result;

			std::map<cfg_var, std::set<cfg_token>> first_sets = chef::first_sets(grammar);
			std::map<cfg_var, std::set<cfg_token>> follow_sets
				= chef::follow_sets(grammar, first_sets);

			for (const cfg_var& var : grammar.vars()) {
				for (const cfg_seq& rule : grammar.rules(var)) {
					for (const cfg_token& token :
						chef::first_plus_set(var, rule, first_sets, follow_sets)) {
						if (!result[var].emplace(token, rule).second) {
							throw std::invalid_argument("Grammar is not LL(1)!");
						}
					}
				}
			}

			return result;
		}
	}

	ll1_table::ll1_table(const cfg& grammar)
		: ll1_table(compute_ll1_table(grammar))
	{ }
}
