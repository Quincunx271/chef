#pragma once

#include <cassert>
#include <cstdint>
#include <span>
#include <utility>
#include <vector>

#include <chef/dfa/dfa.hpp>

namespace chef {
	class nfa {
	private:
		// A 2d array.
		std::vector<std::vector<state_type>> transition_table_;
		state_type num_states_;
		symbol_type num_symbols_;

	public:
		explicit nfa(
			state_type num_states, symbol_type num_symbols, std::vector<fa_edge> const& edge_list)
			: num_states_(num_states)
			, num_symbols_(num_symbols)
		{
			transition_table_.resize(num_states * num_symbols);

			for (auto const [from, to, on] : edge_list) {
				at(from, on).push_back(to);
			}
		}

	public:
		auto num_states() const -> state_type
		{
			return num_states_;
		}

		auto process(state_type from, symbol_type on) const -> std::span<state_type const>
		{
			return at(from, on);
		}

	private:
		auto at(state_type from, symbol_type on) const -> std::vector<state_type> const&
		{
			assert(from < num_states_);
			assert(on < num_symbols_);
			std::size_t const index = num_symbols_ * from + on;
			assert(index < transition_table_.size());

			return transition_table_[index];
		}

		auto at(state_type from, symbol_type on) -> std::vector<state_type>&
		{
			return const_cast<std::vector<state_type>&>(std::as_const(*this).at(from, on));
		}
	};
}
