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
				using namespace std::ranges;

				// FNV offset basis, just to give some entropy into the system:
				std::size_t const offset = sizeof(std::size_t) == sizeof(std::uint32_t)
					? 0x811c9dc5
					: 0xcbf29ce484222325ull;
				// FNV prime, just to give some entropy into the system:
				std::size_t const prime = sizeof(std::size_t) == sizeof(std::uint32_t)
					? 0x01000193
					: 0x00000100000001b3ull;

				auto adjusted = views::common(views::transform(
					states, [prime](auto x) -> std::size_t { return prime * (x + 1); }));

				// Note: plus would be likely be a very bad hash because of the Central Limit
				// Theorem. Basically, plus would make collisions _very likely_.
				return std::accumulate(adjusted.begin(), adjusted.end(), offset, std::bit_xor{});
			}
		};

		template <typename K, typename Hash = std::hash<K>>
		using hashset = std::unordered_set<K, Hash>;

		template <typename K>
		using comm_hashset = hashset<K, commutative_set_hash>;

		template <typename K, typename V, typename Hash = std::hash<K>>
		using hashmap = std::unordered_map<K, V, Hash>;

		template <typename K, typename V>
		using comm_hashmap = hashmap<K, V, commutative_set_hash>;

		template <typename IndexType = std::size_t>
		auto transpose(auto init, std::ranges::range auto const& map2d, auto const& insert_f)
		{
			for (IndexType const index : detail::indices<IndexType>(map2d)) {
				for (auto const second : map2d[index]) {
					insert_f(init[second], index);
				}
			}
			return init;
		}

		inline comm_hashset<hashset<chef::state_type>> get_minimization_partitions(
			chef::dfa const& dfa,
			// Each hashset lists the states in that category
			std::vector<hashset<chef::state_type>> const& categories)
		{
			// What are the categories each state belongs to?
			auto const transposed_cats
				= detail::transpose(std::vector<hashset<std::size_t>>(dfa.num_states()), categories,
					[] TL(_1.insert(_2)));

			// How many kinds of categories are there?
			// Simultaneously store which states have the keyed category.
			comm_hashmap<hashset<std::size_t>, hashset<chef::state_type>> unique_cats;
			for (chef::state_type const state : dfa.states()) {
				unique_cats[transposed_cats[state]].insert(state);
			}

			namespace views = std::ranges::views;

			auto values = views::common(views::values(unique_cats));

			return comm_hashset<hashset<chef::state_type>>(
				std::move_iterator(values.begin()), std::move_iterator(values.end()));
		}

		inline std::vector<hashset<chef::state_type>> hopcroft(chef::dfa const& dfa,
			std::vector<std::unordered_set<chef::state_type>> const& categories)
		{
			namespace views = std::ranges::views;
			// Implements DFA minimization by Hopcroft's algorithm.
			// Note: single-letter capital "variable names" (sometimes comments) come from
			// Wikipedia's psuedocode.

			// Form initial partitions
			// P
			comm_hashset<hashset<chef::state_type>> partitions
				= detail::get_minimization_partitions(dfa, categories);
			// W
			comm_hashset<hashset<chef::state_type>> work_partitions = partitions;

			std::vector<hashset<chef::state_type>> new_partitions;

			while (!work_partitions.empty()) {
				// Pop a partition from W
				hashset<chef::state_type> A = std::move(*work_partitions.begin());
				work_partitions.erase(work_partitions.begin());

				for (chef::symbol_type const sym : dfa.symbols()) {
					// States which go into A on sym
					auto X = detail::to_container<std::unordered_set>(
						dfa.states() | views::filter([&] TL(A.contains(dfa.process(_1, sym)))));

					// for-each, but erase sometimes
					std::erase_if(partitions, [&](hashset<chef::state_type> const& Y) {
						if (detail::has_intersect(X, Y)) {
							if (auto diff = detail::difference(Y, X); !diff.empty()) {
								auto intersect = detail::intersect(X, Y);

								if (work_partitions.contains(Y)) {
									work_partitions.erase(Y);
									work_partitions.insert(diff);
									work_partitions.insert(intersect);
								} else {
									work_partitions.insert(
										intersect.size() <= diff.size() ? intersect : diff);
								}

								new_partitions.push_back(std::move(diff));
								new_partitions.push_back(std::move(intersect));
								return true; // Erase this set
							}
						}
						return false;
					});
					partitions.insert(std::move_iterator(new_partitions.begin()),
						std::move_iterator(new_partitions.end()));
					new_partitions.clear();
				}
			}

			// Translate partitions to an indexable form
			std::vector<hashset<chef::state_type>> partitions_vec;
			partitions_vec.reserve(partitions.size());
			std::move(partitions.begin(), partitions.end(), std::back_inserter(partitions_vec));

			if (!partitions_vec.empty()) { // should always be true
				// The state with 0 is the start state.
				// This state must be at index 0 in the vector so that we maintain start == 0.
				auto it = std::find_if(
					partitions_vec.begin(), partitions_vec.end(), [] TL(_1.contains(0)));
				assert(it != partitions_vec.end() && "DFA is in an invalid state");
				std::iter_swap(partitions_vec.begin(), it);
			}

			return partitions_vec;
		}

		inline std::vector<chef::state_type> hopcroft_new_state_map(
			chef::dfa const& dfa, std::vector<hashset<chef::state_type>> const& partitions)
		{
			return detail::transpose<chef::state_type>(
				std::vector<chef::state_type>(dfa.num_states()), partitions, [] TL(_1 = _2));
		}

		inline chef::dfa hopcroft_to_dfa(chef::dfa const& old_dfa,
			chef::state_type const num_states,
			std::vector<hashset<chef::state_type>> const& partitions,
			std::vector<chef::state_type> const& new_state_map)
		{
			// Construct edge list for new DFA
			std::vector<fa_edge> edges;
			for (auto const new_from : detail::indices<chef::state_type>(partitions)) {
				for (chef::symbol_type const sym : old_dfa.symbols()) {
					chef::state_type const old_from = *partitions[new_from].begin();
					chef::state_type const old_to = old_dfa.process(old_from, sym);
					chef::state_type const new_to = new_state_map[old_to];

					edges.push_back(fa_edge{
						.from = new_from,
						.to = new_to,
						.on = sym,
					});
				}
			}

			return chef::dfa(num_states, old_dfa.num_symbols(), std::move(edges));
		}
	}

	/**
	 * \brief Minimizes the DFA into a DFA with the minimum number of states
	 *
	 * \param dfa
	 * \param categories Predefined categories that distinguish states (e.g. final vs. non-final)
	 */
	inline std::pair<chef::dfa, std::vector<std::unordered_set<chef::state_type>>> minimize(
		chef::dfa const& dfa, std::vector<std::unordered_set<chef::state_type>> const& categories)
	{
		using detail::hashset;

		std::vector<hashset<chef::state_type>> const partitions = detail::hopcroft(dfa, categories);

		// Map [old state] -> [new state]
		std::vector<chef::state_type> const new_state_map
			= detail::hopcroft_new_state_map(dfa, partitions);

		// Translate new DFA
		auto new_dfa = detail::hopcroft_to_dfa(
			dfa, static_cast<chef::state_type>(partitions.size()), partitions, new_state_map);

		// Track categories for new DFA
		std::vector<hashset<chef::state_type>> new_categories(categories.size());
		for (std::size_t const cat_index : detail::indices(categories)) {
			std::transform(categories[cat_index].begin(), categories[cat_index].end(),
				std::inserter(new_categories[cat_index], new_categories[cat_index].end()),
				[&] TL(new_state_map[_1]));
		}

		return std::pair{std::move(new_dfa), std::move(new_categories)};
	}
}
