#include "./cfg.hpp"

#include <algorithm>
#include <iterator>
#include <ostream>
#include <ranges>

#include <tl/tl.hpp>

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

	namespace {
		// Creates a group of sets for each variable in the CFG, repeatedly applying f until no more
		// changes happen.
		template <typename F>
		std::map<cfg_var, std::set<cfg_token>> cfg_sets_steady_state(const cfg& cfg, const F& f)
		{
			std::map<cfg_var, std::set<cfg_token>> result;
			for (auto&& var : cfg.vars())
				result[var];

			bool changed = false;

			do {
				changed = false;

				for (const cfg_var& var : cfg.vars()) {
					std::set<cfg_token>& set = result.find(var)->second;
					const std::size_t prev_size = set.size();

					for (const cfg_seq& rule : cfg.rules(var)) {
						f(result, var, rule, set);
					}

					if (prev_size != set.size()) {
						changed |= true;
					}
				}

			} while (changed);

			return result;
		}

		// Given a results CFG sets (e.g. first sets), gives back a function operating on a
		// cfg_seq::value_type which asks whether epsilon is a valid expansion for the item.
		constexpr auto cfg_has_eps_var_f
			= [](const std::map<cfg_var, std::set<cfg_token>>& result) {
				  return [&result](cfg_seq::value_type const& var) {
					  return std::visit(
						  overload{
							  [](cfg_token const& token) -> bool { return token == cfg_epsilon; },
							  [&result](cfg_var const& var) -> bool {
								  return result.find(var)->second.contains(cfg_epsilon);
							  },
						  },
						  var);
				  };
			  };
	}

	std::map<cfg_var, std::set<cfg_token>> first_sets(const cfg& cfg)
	{
		return cfg_sets_steady_state(cfg,
			[&cfg](const std::map<cfg_var, std::set<cfg_token>>& result, const cfg_var&,
				const cfg_seq& rule, std::set<cfg_token>& first_set) {
				const auto has_eps_var = cfg_has_eps_var_f(result);

				auto first_group_end = ranges::find_if_not(rule, has_eps_var);
				if (first_group_end != rule.end()) ++first_group_end;

				ForwardRangeOf<const std::variant<cfg_var, cfg_token>&> auto first_group
					= ranges::subrange(rule.begin(), first_group_end);

				for (const auto& part : first_group) {
					std::visit(overload{
								   [&](const cfg_token& token) {
									   if (token != cfg_epsilon) first_set.insert(token);
								   },
								   [&](const cfg_var& var) {
									   const std::set<cfg_token>& var_firsts
										   = result.find(var)->second;

									   auto var_firsts_without_eps = views::all(var_firsts)
										   | views::filter([] TL(_1 != cfg_epsilon));

									   first_set.insert(var_firsts_without_eps.begin(),
										   var_firsts_without_eps.end());

									   first_set.size();
								   },
							   },
						part);
				}

				if (first_group.end() == rule.end() && has_eps_var(*std::prev(first_group.end()))) {
					first_set.insert(cfg_epsilon);
				}
			});
	}

	std::map<cfg_var, std::set<cfg_token>> follow_sets(
		const cfg&, const std::map<cfg_var, std::set<cfg_token>>&)
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
