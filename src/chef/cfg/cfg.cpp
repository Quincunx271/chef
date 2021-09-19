#include "./cfg.hpp"

#include <algorithm>
#include <iterator>
#include <ostream>
#include <ranges>

#include <chef/_/overload.hpp>

namespace ranges = std::ranges;
namespace views = ranges::views;

using chef::detail::overload;

namespace chef {
	void cfg::add_rule(cfg_rule rule)
	{
		cfg_rule_body& body = rules_[rule.var];
		body.alts.insert(body.alts.end(), std::move_iterator(rule.body.alts.begin()),
			std::move_iterator(rule.body.alts.end()));
	}

	std::map<cfg_var, std::set<cfg_token>> cfg::first_sets() const
	{
		std::map<cfg_var, std::set<cfg_token>> result;
		for (auto&& var : vars())
			result[var];

		constexpr auto get = [](auto&& m, auto&& k) {
			auto it = m.find(k);
			assert(it != m.end());
			return it;
		};

		auto const has_eps = overload{
			[&](cfg_token const& token) -> bool { return is_vanishable(token); },
			[&](cfg_var const& var) -> bool {
				return get(result, var)->second.contains(cfg_epsilon);
			},
		};
		auto const has_eps_var
			= [&](cfg_seq::value_type const& var) { return std::visit(has_eps, var); };

		bool changed = false;

		do {
			changed = false;

			for (const cfg_var& var : vars()) {
				for (const cfg_seq& rule : rules(var)) {
					auto first_group_end = ranges::find_if_not(rule, has_eps_var);
					if (first_group_end != rule.end()) ++first_group_end;

					ForwardRangeOf<const std::variant<cfg_var, cfg_token>&> auto first_group
						= ranges::subrange(rule.begin(), first_group_end);
					std::set<cfg_token>& first_set = result[var];

					for (const auto& part : first_group) {
						changed |= std::visit(
							overload{
								[&](const cfg_token& token) -> bool {
									return first_set.insert(token).second;
								},
								[&](const cfg_var& var) -> bool {
									const std::set<cfg_token>& var_firsts
										= result.find(var)->second;

									const std::size_t prev_size = first_set.size();
									first_set.insert(var_firsts.begin(), var_firsts.end());

									return first_set.size() != prev_size;
								},
							},
							part);
					}
				}
			}
		} while (changed);

		return result;
	}

	std::map<cfg_var, std::set<cfg_token>> cfg::follow_sets() const
	{
		std::map<cfg_var, std::set<cfg_token>> result;

		// TODO: implement this

		return result;
	}

	namespace detail {
		std::ostream& print_to(std::ostream& out, const cfg_var& var)
		{
			return out << var.value;
		}

		std::ostream& print_to(std::ostream& out, const cfg_token& token)
		{
			out << '<';

			if (token == cfg_epsilon)
				out << "ε";
			else
				out << token.token_type;

			return out << '>';
		}

		std::ostream& print_to(std::ostream& out, const cfg_seq& seq)
		{
			for (const auto& item : seq) {
				std::visit([&out](const auto& it) { out << it; }, item);
			}
			return out;
		}

		std::ostream& print_to(std::ostream& out, const cfg_rule& rule)
		{
			out << rule.var << " -> ";

			bool is_not_first = false;
			for (const cfg_seq& alt : rule.body.alts) {
				if (is_not_first) out << " | ";
				is_not_first = true;
				out << alt;
			}

			return out << ';';
		}
	}
}
