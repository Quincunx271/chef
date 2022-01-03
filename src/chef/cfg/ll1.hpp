#pragma once

#include <iterator>
#include <map>
#include <ranges>
#include <vector>

#include <chef/cfg/cfg.hpp>

// The LL(1) parsing state consists of:
// 1. The stream of tokens (allowing us to read one without consuming it).
// 2. A stack of CFG variables and tokens.
//
// The LL(1) parsing algorithm is:
// 1. If the top element of the stack is a token, pop it, passing if it matches the next token
//    in the input stream (advancing the stream) and failing if it does not match.
// 2. Otherwise, pop the variable, and look up in the LL(1) table for the new sequence of CFG
// 	  variables and tokens to push onto the stack.
// Repeat until finished.

namespace chef {
	namespace detail {
		template <typename Container, typename Value = std::ranges::range_value_t<Container>>
		concept InsertableContainer = requires(Container& container,
			std::istream_iterator<Value> input_it, std::vector<Value>::const_iterator rand_it)
		{
			container.insert(container.end(), input_it, input_it);
			container.insert(container.end(), rand_it, rand_it);
		};
	}

	// This table tells us how to expand the LL(1) parsing state given the next token of input.
	class ll1_table {
	private:
		std::map<cfg_var, std::map<cfg_token, cfg_seq>> table_;

	public:
		explicit ll1_table(std::map<cfg_var, std::map<cfg_token, cfg_seq>> table)
			: table_(std::move(table))
		{ }

		template <detail::InsertableContainer<cfg_seq::value_type> Container>
		void expand_variable(
			Container& container, const cfg_var& cur, const cfg_token& next_token) const
		{
			if (const auto var_map_it = table_.find(cur); var_map_it != table_.end()) {
				if (const auto tok_map_it = var_map_it->second.find(next_token);
					tok_map_it != var_map_it->second.end())
				{
					container.insert(
						container.end(), tok_map_it->second.begin(), tok_map_it->second.end());
				}
			}
		}
	};
}
