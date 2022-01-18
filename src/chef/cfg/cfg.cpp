#include "./cfg.hpp"

#include <algorithm>
#include <iterator>
#include <ostream>
#include <ranges>

#include <tl/tl.hpp>

#include <chef/_/fn.hpp>
#include <chef/_/fwd.hpp>
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
		std::map<cfg_var, std::set<cfg_token>> cfg_default_init_sets(const cfg& cfg)
		{
			std::map<cfg_var, std::set<cfg_token>> result;
			for (auto&& var : cfg.vars())
				result[var];

			return result;
		}

		// Creates a group of sets for each variable in the CFG, repeatedly applying f until no more
		// changes happen.
		template <typename F>
		std::map<cfg_var, std::set<cfg_token>> cfg_sets_steady_state(
			const cfg& cfg, std::map<cfg_var, std::set<cfg_token>> result, const F& f)
		{
			bool changed = false;

			do {
				changed = false;

				for (const cfg_var& var : cfg.vars()) {
					for (const cfg_seq& rule : cfg.rules(var)) {
						changed |= f(result, var, rule);
					}
				}
			} while (changed);

			return result;
		}
	}

	std::map<cfg_var, std::set<cfg_token>> first_sets(const cfg& cfg)
	{
		// Given a results CFG sets (e.g. first sets), gives back a function operating on a
		// cfg_seq::value_type which asks whether epsilon is a valid expansion for the item.
		static constexpr auto cfg_has_eps_var_f
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

		return cfg_sets_steady_state(cfg, cfg_default_init_sets(cfg),
			[&cfg](std::map<cfg_var, std::set<cfg_token>>& result, const cfg_var& var,
				const cfg_seq& rule) {
				std::set<cfg_token>& first_set = result.find(var)->second;
				const std::size_t prev_size = first_set.size();

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

				return prev_size != first_set.size();
			});
	}

	std::map<cfg_var, std::set<cfg_token>> follow_sets(
		const cfg& cfg, const std::map<cfg_var, std::set<cfg_token>>& first_sets)
	{
		const auto first_set_of
			= [&first_sets](const cfg_var& var) { return first_sets.find(var)->second; };
		const auto first_has_eps = [&first_set_of](const cfg_var& var) {
			return first_set_of(var).contains(cfg_epsilon);
		};
		const auto first_has_eps_var = [&](const cfg_seq::value_type& var) {
			return std::visit(overload{
								  [&](const cfg_var& var) { return first_has_eps(var); },
								  [](const cfg_token tok) { return tok == cfg_epsilon; },
							  },
				var);
		};

		auto follow_sets = cfg_default_init_sets(cfg);
		follow_sets[cfg.start_var()].insert(cfg_eof);

		return cfg_sets_steady_state(cfg, CHEF_MOVE(follow_sets),
			[&](std::map<cfg_var, std::set<cfg_token>>& result, const cfg_var& var_from,
				const cfg_seq& rule) {
				bool changed = false;

				for (auto it = rule.begin(); it != rule.end(); ++it) {
					if (!std::holds_alternative<cfg_var>(*it)) continue;

					std::set<cfg_token>& follow_set
						= result.find(*std::get_if<cfg_var>(&*it))->second;
					const std::size_t prev_size = follow_set.size();
					auto remaining = ranges::subrange(it + 1, rule.end());

					if (remaining.empty()) {
						auto const& var_from_follows = result.find(var_from)->second;
						follow_set.insert(var_from_follows.begin(), var_from_follows.end());
					}
					for (auto const& seq_item : remaining) {
						std::visit(overload{
									   [&](const cfg_var& var) {
										   auto const& new_var_firsts = first_set_of(var);
										   follow_set.insert(
											   new_var_firsts.begin(), new_var_firsts.end());
										   follow_set.erase(cfg_epsilon);
									   },
									   [&](const cfg_token tok) {
										   if (tok != cfg_epsilon) {
											   follow_set.insert(tok);
										   }
									   },
								   },
							seq_item);
						if (!first_has_eps_var(seq_item)) break;
					}

					changed |= follow_set.size() != prev_size;
				}

				return changed;
			});
	}

	std::set<cfg_token> first_plus_set(const cfg_var& var, const cfg_seq& rule,
		const std::map<cfg_var, std::set<cfg_token>>& first_sets,
		const std::map<cfg_var, std::set<cfg_token>>& follow_sets)
	{
		std::set<cfg_token> result;

		bool all_erasable = false;
		for (const auto& item : rule) {
			const bool erasable = std::visit(detail::overload{
												 [&](cfg_token tok) {
													 result.insert(tok);
													 return false;
												 },
												 [&](const cfg_var& var) {
													 const std::set<cfg_token>& first
														 = first_sets.find(var)->second;
													 result.insert(first.begin(), first.end());
													 result.erase(cfg_epsilon);
													 return first.contains(cfg_epsilon);
												 },
											 },
				item);
			all_erasable |= erasable;
			if (!erasable) break;
		}

		if (all_erasable) {
			const std::set<cfg_token>& follow = follow_sets.find(var)->second;
			result.insert(follow.begin(), follow.end());
		}

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
