#pragma once

#include <algorithm>
#include <iterator>
#include <map>
#include <optional>
#include <ranges>
#include <vector>

#include <chef/cfg/ast.hpp>
#include <chef/cfg/cfg.hpp>
#include <chef/util/ranges.hpp>

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

		explicit ll1_table(const cfg& grammar);

		// Expand the (variable, token) sequence into the provided container.
		//
		// Note that the expanded cfg_seq will be in reverse order, like a stack.
		//
		// Parameters:
		//  - container: a container with an .insert() function to expand the variable into.
		//  - cur: the cfg_var to expand.
		//  - next_token: which token to expand the cfg_var for.
		//
		// Returns:
		//  - whether the variable could be expanded. If this returns false,
		//    then this variable cannot be expanded with this token, meaning that
		//    parsing should fail.
		template <detail::InsertableContainer<cfg_seq::value_type> Container>
		bool expand_variable(
			Container& container, const cfg_var& cur, const cfg_token& next_token) const
		{
			if (const auto var_map_it = table_.find(cur); var_map_it != table_.end()) {
				if (const auto tok_map_it = var_map_it->second.find(next_token);
					tok_map_it != var_map_it->second.end())
				{
					container.insert(
						container.end(), tok_map_it->second.rbegin(), tok_map_it->second.rend());
					// `next_token` is set in the table.
					return true;
				} else {
					// Parsing failed; `next_token` is not set.
					return false;
				}
			}
			assert(false && "Every variable must be accounted for in the table.");
		}

		template <InputRangeOf<const cfg_token&> TokenStream>
		bool parse(cfg_var start, TokenStream&& tokens) const
		{
			using std::ranges::begin;
			using std::ranges::end;

			std::vector<cfg_seq::value_type> stack;
			stack.push_back(std::move(start));

			for (auto first = begin(tokens); first != end(tokens);) {
				if (stack.empty()) {
					// An empty stack only matches the empty string, but we have at least one more
					// token to process.
					return false;
				}
				cfg_seq::value_type cur = std::move(stack.back());
				stack.pop_back();

				// Either forms a reference or performs temporary lifetime extension if the
				// operator*() actually returns an rvalue.
				const cfg_token& next = *first;

				if (std::holds_alternative<cfg_token>(cur)) {
					// Raw tokens must match exactly.
					if (std::get<cfg_token>(cur) != next) return false;
					++first;
				} else {
					// Variables are expanded according to the next token, and we do not pop from
					// the input sequence.
					expand_variable(stack, std::get<cfg_var>(cur), next);
				}
			}

			return stack.empty();
		}

		template <InputRangeOf<const cfg_token&> TokenStream>
		std::optional<rt_ast::ast_node> parse_rt(cfg_var start, TokenStream&& tokens) const
		{
			using std::ranges::begin;
			using std::ranges::end;

			struct rule_end_sentinel { };

			std::vector<std::variant<rule_end_sentinel, cfg_seq::value_type>> stack;
			stack.push_back(std::move(start));

			std::vector<rt_ast::ast_node> seq_stack;
			seq_stack.emplace_back().name = start.value;

			for (auto first = begin(tokens); first != end(tokens);) {
				if (stack.empty()) {
					// An empty stack only matches the empty string, but we have at least one more
					// token to process.
					return std::nullopt;
				}
				auto cur = std::move(stack.back());
				stack.pop_back();

				// Either forms a reference or performs temporary lifetime extension if the
				// operator*() actually returns an rvalue.
				const cfg_token& next = *first;

				if (std::holds_alternative<rule_end_sentinel>(cur)) {
					auto node = std::make_unique<rt_ast::ast_node>(std::move(seq_stack.back()));
					seq_stack.pop_back();
					seq_stack.back().children.push_back(std::move(node));
				} else {
					auto actual_cur = std::get<cfg_seq::value_type>(std::move(cur));
					if (std::holds_alternative<cfg_token>(actual_cur)) {
						// Raw tokens must match exactly.
						if (std::get<cfg_token>(actual_cur) != next) return std::nullopt;
						seq_stack.back().children.push_back(std::get<cfg_token>(actual_cur));
						++first;
					} else {
						stack.emplace_back(rule_end_sentinel{});
						seq_stack.emplace_back().name = std::get<cfg_var>(actual_cur).value;
						// Variables are expanded according to the next token, and we do not pop
						// from the input sequence.
						std::vector<cfg_seq::value_type> extract;
						expand_variable(extract, std::get<cfg_var>(actual_cur), next);
						std::ranges::transform(extract, std::back_inserter(stack),
							[](cfg_seq::value_type& val) -> decltype(stack)::value_type {
								return std::move(val);
							});
					}
				}
			}

			while (!stack.empty() && std::holds_alternative<rule_end_sentinel>(stack.back())) {
				stack.pop_back();
				auto node = std::make_unique<rt_ast::ast_node>(std::move(seq_stack.back()));
				seq_stack.pop_back();
				seq_stack.back().children.push_back(std::move(node));
			}

			if (!stack.empty()) {
				return std::nullopt;
			}
			assert(seq_stack.size() == 1);
			return std::move(
				*std::get<std::unique_ptr<rt_ast::ast_node>>(seq_stack.front().children[0]));
		}
	};
}
