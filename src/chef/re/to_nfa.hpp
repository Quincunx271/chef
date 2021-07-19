#pragma once

#include <algorithm>
#include <functional>
#include <iterator>
#include <numeric>
#include <ranges>
#include <unordered_set>
#include <utility>
#include <vector>

#include <chef/_/fwd.hpp>
#include <chef/_/overload.hpp>
#include <chef/dfa/nfa.hpp>
#include <chef/re/re.hpp>
#include <tl/tl.hpp>

namespace chef {
	namespace detail {
		inline std::pair<nfa, std::unordered_set<chef::state_type>> to_nfa(
			chef::re const& re, std::unordered_map<char, chef::symbol_type> const& symbol_map)
		{
			const chef::symbol_type num_symbols
				= static_cast<chef::symbol_type>(symbol_map.size()) + 1;

			return std::visit(
				detail::overload{
					[&](re_union const& re) {
						std::vector<std::pair<nfa, std::unordered_set<chef::state_type>>> nfas;
						nfas.reserve(re.pieces.size());
						std::ranges::transform(re.pieces,
							std::back_inserter(nfas), [&] TL(detail::to_nfa(*_1, symbol_map)));

						const chef::state_type num_states = 1
							+ std::transform_reduce(nfas.begin(), nfas.end(), chef::state_type(0),
								std::plus<>{}, [] TL(_1.first.num_states()));

						std::vector<fa_edge> edge_list;

						std::unordered_set<chef::state_type> accepts;
						accepts.reserve(std::transform_reduce(nfas.begin(), nfas.end(),
							std::size_t(0), std::plus<>{}, [] TL(_1.second.size())));

						chef::state_type offset = 1; // state 0 is the initial state
						for (auto const& [cur, cur_accepts] : nfas) {
							edge_list.push_back(fa_edge{
								.from = 0,
								.to = offset, // this alternative's "initial" state
								.on = chef::nfa::eps,
							});
							for (const chef::state_type from : cur.states()) {
								for (const chef::symbol_type sym : cur.symbols()) {
									for (const chef::state_type to : cur.process(from, sym)) {
										edge_list.push_back(fa_edge{
											.from = from + offset,
											.to = to + offset,
											.on = sym,
										});
									}
								}
							}

							std::ranges::transform(cur_accepts,
								std::inserter(accepts, accepts.end()), [=] TL(_1 + offset));

							offset += cur.num_states();
						}

						return std::pair{
							nfa(num_states, num_symbols, edge_list),
							CHEF_MOVE(accepts),
						};
					},
					[&](re_cat const& re) {
						std::vector<std::pair<nfa, std::unordered_set<chef::state_type>>> nfas;
						nfas.reserve(re.pieces.size());
						std::ranges::transform(re.pieces,
							std::back_inserter(nfas), [&] TL(detail::to_nfa(*_1, symbol_map)));

						const chef::state_type num_states
							= std::transform_reduce(nfas.begin(), nfas.end(), chef::state_type(0),
								std::plus<>{}, [] TL(_1.first.num_states()));

						std::vector<fa_edge> edge_list;

						std::unordered_set<chef::state_type> last_accepts;

						chef::state_type offset = 0; // state 0 is the initial state
						for (auto&& [cur, cur_accepts] : nfas) {
							std::ranges::transform(last_accepts, std::back_inserter(edge_list),
								[=] TL(fa_edge{
									.from = _1,
									.to = offset, // this section's "initial" state
									.on = chef::nfa::eps,
								}));

							for (const chef::state_type from : cur.states()) {
								for (const chef::symbol_type sym : cur.symbols()) {
									for (const chef::state_type to : cur.process(from, sym)) {
										edge_list.push_back(fa_edge{
											.from = from + offset,
											.to = to + offset,
											.on = sym,
										});
									}
								}
							}

							last_accepts.clear();
							std::ranges::transform(cur_accepts,
								std::inserter(last_accepts, last_accepts.end()),
								[=] TL(_1 + offset));

							offset += cur.num_states();
						}

						return std::pair{
							nfa(num_states, num_symbols, edge_list),
							CHEF_MOVE(last_accepts),
						};
					},
					[&](re_star const& re) {
						auto [value, accepts] = detail::to_nfa(*re.value, symbol_map);

						const chef::state_type num_states = 2 + value.num_states();
						const chef::state_type last = num_states - 1;

						std::vector<fa_edge> edge_list;

						edge_list.push_back(fa_edge{
							.from = 0,
							.to = 1,
							.on = chef::nfa::eps,
						});
						edge_list.push_back(fa_edge{
							.from = 0,
							.to = last,
							.on = chef::nfa::eps,
						});

						for (const chef::state_type from : value.states()) {
							for (const chef::symbol_type sym : value.symbols()) {
								for (const chef::state_type to : value.process(from, sym)) {
									edge_list.push_back(fa_edge{
										.from = from + 1,
										.to = to + 1,
										.on = sym,
									});
								}
							}
						}

						for (const chef::state_type accept : accepts) {
							// The back edge
							edge_list.push_back(fa_edge{
								.from = accept + 1,
								.to = 1,
								.on = chef::nfa::eps,
							});
							edge_list.push_back(fa_edge{
								.from = accept + 1,
								.to = last,
								.on = chef::nfa::eps,
							});
						}

						return std::pair{
							nfa(num_states, num_symbols, edge_list),
							std::unordered_set({last}),
						};
					},
					[&](re_lit const& re) {
						const chef::state_type num_states = chef::state_type(re.value.size()) + 1;

						std::vector<fa_edge> edge_list;
						edge_list.reserve(re.value.size());

						for (auto const index : detail::indices<chef::state_type>(re.value)) {
							edge_list.push_back(fa_edge{
								.from = index,
								.to = index + 1,
								.on = symbol_map.at(re.value[index]) + 1,
							});
						}

						return std::pair{
							nfa(num_states, num_symbols, edge_list),
							std::unordered_set({chef::state_type(num_states - 1)}),
						};
					},
					[&](re_empty) {
						return std::pair{
							nfa(1, num_symbols, {}),
							std::unordered_set<chef::state_type>(),
						};
					},
					[&](re_char_class) -> std::pair<nfa, std::unordered_set<chef::state_type>> {
						throw 1;
					},
				},
				re.value);
		}
	}

	struct nfa_conversion_result_t {
		chef::nfa nfa;
		std::unordered_set<chef::state_type> accepts;
		std::unordered_map<char, chef::symbol_type> symbol_map;
	};

	inline nfa_conversion_result_t to_nfa(chef::re const& re)
	{
		auto symbol_map = re.accumulate_chars(std::unordered_map<char, chef::symbol_type>(),
			[](std::unordered_map<char, chef::symbol_type> acc, char const val) {
				acc.emplace(val, acc.size());
				return acc;
			});

		auto [nfa, accepts] = detail::to_nfa(re, symbol_map);
		return nfa_conversion_result_t{
			.nfa = CHEF_MOVE(nfa),
			.accepts = CHEF_MOVE(accepts),
			.symbol_map = CHEF_MOVE(symbol_map),
		};
	}
}
