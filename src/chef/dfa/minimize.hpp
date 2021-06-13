#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <numeric>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <tl/tl.hpp>

#include <chef/_/ranges.hpp>
#include <chef/_/set.hpp>
#include <chef/dfa/dfa.hpp>

namespace chef {
	namespace detail {
		struct commutative_set_hash {
			template <std::convertible_to<std::size_t> T>
			std::size_t operator()(std::unordered_set<T> const& states) const
			{
				// FNV offset basis:
				std::size_t const offset = sizeof(std::size_t) == sizeof(std::uint32_t)
					? 0x811c9dc5
					: 0xcbf29ce484222325ull;

				return std::accumulate(states.begin(), states.end(), offset, std::plus{});
			}
		};

		std::unordered_set<std::unordered_set<chef::state_type>, commutative_set_hash>
		get_minimization_partitions(chef::dfa const& dfa,
			std::vector<std::unordered_set<chef::state_type>> const& categories)
		{
			// Maps [state] -> category
			std::vector<std::unordered_set<std::size_t>> transposed_cats(dfa.num_states());
			for (std::size_t index = 0; index < categories.size(); ++index) {
				for (chef::state_type const state : categories[index]) {
					transposed_cats[state].insert(index);
				}
			}

			std::unordered_map<std::unordered_set<std::size_t>,
				std::unordered_set<chef::state_type>, commutative_set_hash>
				unique_cats;
			for (chef::state_type const state : dfa.states()) {
				unique_cats[transposed_cats[state]].insert(state);
			}

			auto values = std::ranges::views::common(std::ranges::views::values(unique_cats));

			return std::unordered_set<std::unordered_set<chef::state_type>, commutative_set_hash>(
				std::make_move_iterator(values.begin()), std::make_move_iterator(values.end()));
		}
	}

	/**
	 * \brief Minimizes the DFA into a DFA with the minimum number of states
	 *
	 * \param dfa
	 * \param categories Predefined categories that distinguish states (e.g. final vs. non-final)
	 */
	std::pair<chef::dfa, std::vector<std::unordered_set<chef::state_type>>> minimize(
		chef::dfa const& dfa, std::vector<std::unordered_set<chef::state_type>> const& categories)
	{
		namespace views = std::ranges::views;
		// Implements DFA minimization by Hopcroft's algorithm.

		// Form initial partitions
		// P
		std::unordered_set<std::unordered_set<chef::state_type>, detail::commutative_set_hash>
			partitions = detail::get_minimization_partitions(dfa, categories);
		partitions.reserve(dfa.num_states() + 5); // Impl. relies on iterator stability.
		// W
		std::unordered_set<std::unordered_set<chef::state_type>, detail::commutative_set_hash>
			work_partitions = partitions;

		// While there is still work to do
		{
			std::vector<std::unordered_set<chef::state_type>> new_partitions;

			while (!work_partitions.empty()) {
				std::unordered_set<chef::state_type> A = std::move(*work_partitions.begin());
				work_partitions.erase(work_partitions.begin());

				for (chef::symbol_type const sym : dfa.symbols()) {
					auto X = detail::to_container<std::unordered_set>(
						dfa.states() | views::filter([&] TL(A.contains(dfa.process(_1, sym)))));

					new_partitions.clear();
					for (auto y_it = partitions.begin(); y_it != partitions.end();) {
						if (detail::has_intersect(X, *y_it)) {
							if (auto diff = detail::difference(*y_it, X); !diff.empty()) {
								auto intersect = detail::intersect(X, *y_it);

								if (work_partitions.contains(*y_it)) {
									work_partitions.erase(*y_it);
									work_partitions.insert(diff);
									work_partitions.insert(intersect);
								} else {
									if (intersect.size() <= diff.size()) {
										work_partitions.insert(intersect);
									} else {
										work_partitions.insert(diff);
									}
								}

								y_it = partitions.erase(y_it);
								new_partitions.push_back(std::move(diff));
								new_partitions.push_back(std::move(intersect));
							} else {
								++y_it;
							}
						} else {
							++y_it;
						}
					}
					partitions.insert(std::move_iterator(new_partitions.begin()),
						std::move_iterator(new_partitions.end()));
				}
			}
		}

		std::vector<std::unordered_set<chef::state_type>> partitions_vec;
		partitions_vec.reserve(partitions.size());
		std::move(partitions.begin(), partitions.end(), std::back_inserter(partitions_vec));

		if (!partitions_vec.empty()) { // should always be true
			// The state with 0 is the start state.
			// This state must be at index 0 in the vector so that we maintain start == 0.
			auto it
				= std::find_if(partitions_vec.begin(), partitions_vec.end(), [] TL(_1.contains(0)));
			assert(it != partitions_vec.end() && "DFA is in an invalid state");
			std::iter_swap(partitions_vec.begin(), it);
		}

		std::unordered_map<chef::state_type, chef::state_type> new_state_map;
		new_state_map.reserve(dfa.num_states());
		for (chef::state_type const new_state :
			views::iota(chef::state_type(0), static_cast<chef::state_type>(partitions_vec.size())))
		{
			for (chef::state_type const orig : partitions_vec[new_state]) {
				new_state_map.emplace(orig, new_state);
			}
		}

		std::vector<fa_edge> edges;
		for (chef::state_type const new_from :
			views::iota(chef::state_type(0), static_cast<chef::state_type>(partitions_vec.size())))
		{
			for (chef::symbol_type const sym : dfa.symbols()) {
				chef::state_type const old_from = *partitions_vec[new_from].begin();
				chef::state_type const old_to = dfa.process(old_from, sym);
				chef::state_type const new_to = new_state_map.find(old_to)->second;

				edges.push_back(fa_edge{
					.from = new_from,
					.to = new_to,
					.on = sym,
				});
			}
		}

		std::vector<std::unordered_set<chef::state_type>> new_categories(categories.size());
		for (std::size_t const cat_index : detail::indices(categories)) {
			std::transform(categories[cat_index].begin(), categories[cat_index].end(),
				std::inserter(new_categories[cat_index], new_categories[cat_index].end()),
				[&] TL(new_state_map.find(_1)->second));
		}

		return std::pair{
			chef::dfa(static_cast<chef::state_type>(partitions_vec.size()), dfa.num_symbols(),
				std::move(edges)),
			std::move(new_categories),
		};
	}
}
