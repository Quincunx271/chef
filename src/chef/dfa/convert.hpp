#pragma once

#include <algorithm>
#include <iterator>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <chef/dfa/dfa.hpp>
#include <chef/dfa/nfa.hpp>

namespace chef {
	std::pair<chef::dfa, std::vector<std::unordered_set<chef::state_type>>> to_dfa(
		chef::nfa const& nfa, std::vector<std::unordered_set<chef::state_type>> const& categories)
	{
		using mstate_type = std::vector<chef::state_type>;
		std::set<mstate_type> discovered_states({mstate_type({0})});

		std::unordered_map<mstate_type const*, std::vector<mstate_type const*>> transition_table;
		std::unordered_map<mstate_type const*, state_type> state_numbers;

		// Do the conversion algorithm
		{
			// Using a stack for the queue rather than a FIFO queue.
			std::vector<mstate_type const*> processing_queue({&*discovered_states.begin()});
			state_numbers.emplace(processing_queue.back(), 0);

			// Outside loop to reduce allocation.
			mstate_type next;
			do {
				mstate_type const* cur = processing_queue.back();
				processing_queue.pop_back();

				for (symbol_type symbol = 0; symbol < nfa.num_symbols(); ++symbol) {
					for (state_type state : *cur) {
						auto inext = nfa.process(state, symbol);
						next.insert(next.end(), inext.begin(), inext.end());
					}

					mstate_type actual_next;
					std::sort(next.begin(), next.end());
					std::unique_copy(next.begin(), next.end(), std::back_inserter(actual_next));
					next.clear();

					auto const [it, inserted] = discovered_states.emplace(std::move(actual_next));
					if (inserted) {
						processing_queue.push_back(&*it);
						state_numbers.emplace(&*it, state_numbers.size());
					}
					transition_table[cur].push_back(&*it);
				}
			} while (!processing_queue.empty());
		}

		// Gather the DFA transition info into the DFA format
		std::vector<fa_edge> edges;
		edges.reserve(discovered_states.size() * nfa.num_symbols());

		for (auto const& [from, transitions] : transition_table) {
			for (symbol_type symbol = 0; symbol < nfa.num_symbols(); ++symbol) {
				edges.push_back(fa_edge{
					.from = state_numbers.find(from)->second,
					.to = state_numbers.find(transitions[symbol])->second,
					.on = symbol,
				});
			}
		}

		// Trace categories
		std::vector<std::unordered_set<chef::state_type>> dfa_categories;
		dfa_categories.resize(categories.size());
		for (std::size_t i = 0; i < categories.size(); ++i) {
			for (mstate_type const& mstate : discovered_states) {
				if (std::any_of(mstate.begin(), mstate.end(), [&](chef::state_type const state) {
						return categories[i].contains(state);
					}))
				{
					dfa_categories[i].insert(state_numbers.find(&mstate)->second);
				}
			}
		}

		return std::pair{
			chef::dfa(discovered_states.size(), nfa.num_symbols(), edges),
			std::move(dfa_categories),
		};
	}
}
