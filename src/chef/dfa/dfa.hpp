#pragma once

#include <cassert>
#include <utility>
#include <vector>

#include <chef/dfa/fa.hpp>

namespace chef {
	class dfa {
	private:
		// A 2d array.
		std::vector<state_type> transition_table_;
		state_type num_states_;
		symbol_type num_symbols_;

	public:
		explicit dfa(
			state_type num_states, symbol_type num_symbols, std::vector<fa_edge> const& edge_list)
			: num_states_(num_states)
			, num_symbols_(num_symbols)
		{
			assert(edge_list.size() == num_states * num_symbols);
			transition_table_.resize(num_states * num_symbols);

			for (auto const [from, to, on] : edge_list) {
				at(from, on) = to;
			}
		}

	public:
		auto num_states() const -> state_type
		{
			return num_states_;
		}

		auto num_symbols() const -> symbol_type
		{
			return num_symbols_;
		}

		auto process(state_type from, symbol_type on) const -> state_type
		{
			return at(from, on);
		}

	private:
		auto at(state_type from, symbol_type on) const -> state_type const&
		{
			assert(from < num_states_);
			assert(on < num_symbols_);
			std::size_t const index = num_symbols_ * from + on;
			assert(index < transition_table_.size());

			return transition_table_[index];
		}

		auto at(state_type from, symbol_type on) -> state_type&
		{
			return const_cast<state_type&>(std::as_const(*this).at(from, on));
		}
	};
}
