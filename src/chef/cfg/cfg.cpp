#include "./cfg.hpp"

#include <iterator>

namespace chef {
	void cfg::add_rule(cfg_rule rule)
	{
		cfg_rule_body& body = rules_[rule.var];
		body.alts.insert(body.alts.end(), std::move_iterator(rule.body.alts.begin()),
			std::move_iterator(rule.body.alts.end()));
	}
}
