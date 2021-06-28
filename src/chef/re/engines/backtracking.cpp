#include "./backtracking.hpp"

#include <cassert>
#include <optional>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <tl/tl.hpp>

#include <chef/_/one_of.hpp>
#include <chef/_/overload.hpp>
#include <chef/_/ranges.hpp>

using chef::detail::DecaysOneOf;
using chef::detail::overload;

namespace {
	struct engine_builder {
		std::unordered_map<chef::re const*, std::vector<chef::re const*>> next;
		std::unordered_map<chef::re const*, std::vector<chef::re const*>> back;
		std::unordered_set<chef::re const*> accept;
		// The last RE pieces associated with the RE.
		// If not in the map, then it is itself the "last".
		std::unordered_map<chef::re const*, std::vector<chef::re const*>> last_piece;
		// Which RE pieces should have their next/backs populated by me
		std::unordered_map<chef::re const*, std::vector<chef::re const*>> dependent_next_back;

		explicit engine_builder(chef::re const& re)
		{
			make_lasts(&re);
			// Find forward (cat) and backward edges (star) between pieces
			make_next_back_edges(&re);
			for_each_last(&re, [&](chef::re const* last) { accept.insert(last); });
		}

	private:
		void add_next(chef::re const* re, chef::re const* next)
		{
			this->next[re].push_back(next);

			if (auto it = dependent_next_back.find(re); it != dependent_next_back.end()) {
				for (chef::re const* dependent : it->second) {
					add_next(dependent, next);
				}
			}
		}

		void add_back(chef::re const* re, chef::re const* back)
		{
			this->back[re].push_back(back);

			if (auto it = dependent_next_back.find(re); it != dependent_next_back.end()) {
				for (chef::re const* dependent : it->second) {
					add_back(dependent, back);
				}
			}
		}

		template <typename F>
		void for_each_last(chef::re const* re_p, F&& f)
		{
			auto m_it = last_piece.find(re_p);
			if (m_it == last_piece.end()) {
				f(re_p);
				return;
			}

			for (chef::re const* last_p : m_it->second) {
				for_each_last(last_p, f);
			}
		}

		void make_next_back_edges(chef::re const* re_p)
		{
			std::visit([&](auto const& re) { make_next_back_edges(re_p, re); }, re_p->value);
		}

		void make_next_back_edges(chef::re const*, chef::re_union const& re)
		{
			for (auto const& sub : re.pieces) {
				make_next_back_edges(sub.get());
			}
		}

		void make_next_back_edges(chef::re const*, chef::re_cat const& re)
		{
			for (auto const& sub : re.pieces) {
				make_next_back_edges(sub.get());
			}
			if (!re.pieces.empty()) {
				for (auto first = re.pieces.begin(), second = re.pieces.begin() + 1;
					 second != re.pieces.end(); ++first, ++second)
				{
					for_each_last(
						first->get(), [&](chef::re const* last) { add_next(last, second->get()); });
				}
			}
		}

		void make_next_back_edges(chef::re const* re_p, chef::re_star const& re)
		{
			make_next_back_edges(re.value.get());
			for_each_last(re.value.get(), [&](chef::re const* last) { add_back(last, re_p); });
		}

		void make_next_back_edges(chef::re const*,
			DecaysOneOf<chef::re_lit, chef::re_empty, chef::re_char_class> auto const&)
		{ }

		void make_lasts(chef::re const* re_p)
		{
			std::visit([&](auto const& re) { make_lasts(re_p, re); }, re_p->value);
		}

		void make_lasts(chef::re const* re_p, chef::re_union const& re)
		{
			for (auto const& sub : re.pieces) {
				auto& lasts = last_piece[re_p];
				lasts.push_back(sub.get());
			}
			for (auto const& sub : re.pieces) {
				make_lasts(sub.get());
			}
		}

		void make_lasts(chef::re const* re_p, chef::re_cat const& re)
		{
			if (!re.pieces.empty()) {
				auto& lasts = last_piece[re_p];
				lasts.push_back(re.pieces.back().get());
			}
			for (auto const& sub : re.pieces) {
				make_lasts(sub.get());
			}
		}

		void make_lasts(chef::re const* re_p, chef::re_star const& re)
		{
			dependent_next_back[re.value.get()].push_back(re_p);

			last_piece[re_p].push_back(re.value.get());
			make_lasts(re.value.get());
		}

		void make_lasts(chef::re const*,
			DecaysOneOf<chef::re_lit, chef::re_empty, chef::re_char_class> auto const&)
		{ }
	};

	class engine {
		std::unordered_map<chef::re const*, std::vector<chef::re const*>> next;
		std::unordered_map<chef::re const*, std::vector<chef::re const*>> back;
		std::unordered_set<chef::re const*> accept;
		chef::re const* re;

	private:
		explicit engine(chef::re const& re, engine_builder builder)
			: next(std::move(builder.next))
			, back(std::move(builder.back))
			, accept(std::move(builder.accept))
			, re(&re)
		{ }

	public:
		explicit engine(chef::re const& re)
			: engine(re, engine_builder(re))
		{ }

		bool matches(std::string_view str) const
		{
			struct state {
				chef::re const* cur;
				std::string_view remaining;
				std::size_t index = 0;
				std::ptrdiff_t finished_index = -1; // after we've already finished, evaluate nexts.
			};

			// We will walk the NFA for the regex as a DFS.

			std::stack<state> stack;
			stack.emplace(re, str);
			// To avoid infinite loops of epsilon transitions, don't take the back edge if we
			// haven't moved since we were last in that state.
			std::unordered_map<chef::re const*, std::string_view> last_remaining;

			do {
				state& cur = stack.top();
				last_remaining.insert_or_assign(cur.cur, cur.remaining);
				if (cur.remaining.empty() && accept.contains(cur.cur)) return true;

				auto const backtrack = [&] {
					last_remaining.erase(cur.cur);
					stack.pop();
				};

				if (cur.finished_index >= 0) {
					// Try nexts first:
					std::size_t index = cur.finished_index;
					if (auto it = next.find(cur.cur); it != next.end()) {
						if (index < it->second.size()) {
							++cur.finished_index;
							stack.emplace(it->second[index], cur.remaining);
							continue;
						} else {
							index -= it->second.size(); // adjust for `back`
						}
					}
					if (auto it = back.find(cur.cur); it != back.end() && index < it->second.size())
					{
						for (auto last = last_remaining.find(it->second[index]);
							 last != last_remaining.end() && last->second == cur.remaining;
							 last = last_remaining.find(it->second[index]))
						{
							// We can't backtrack to here, as it would lead to an infinite loop of
							// epsilon transitions. Skip this and try the next `back`.
							++index;
							++cur.finished_index;

							if (index >= it->second.size()) {
								backtrack();
								continue;
							}
						}
						++cur.finished_index;
						stack.emplace(it->second[index], cur.remaining);
						continue;
					}
					// If we didn't find any option, backtrack
					backtrack();
				}

				std::visit(
					overload{
						[&](chef::re_union const& re) {
							if (cur.index >= re.pieces.size()) {
								cur.finished_index = 0; // mark that we finished evaluating this RE.
								return;
							}

							++cur.index;
							stack.emplace(re.pieces[cur.index - 1].get(), cur.remaining);
						},
						[&](chef::re_cat const& re) {
								cur.finished_index = 0; // mark that we finished evaluating this RE.
							// if (cur.index >= re.pieces.size()) {
							// 	return;
							// }

							// ++cur.index;
							stack.emplace(re.pieces.front().get(), cur.remaining);
						},
						[&](chef::re_star const& re) {
							// TODO: Fix this: if we backtrack to here, we unconditionally try again...
							cur.finished_index = 0; // Maybe this fixes it?
							stack.emplace(re.value.get(), cur.remaining);
						},
						[&](chef::re_lit const& re) {
							if (!cur.remaining.starts_with(re.value)) {
								backtrack();
								return;
							}
							cur.remaining.remove_prefix(re.value.size());

							cur.finished_index = 0; // mark that we finished evaluating this RE.
							return;
						},
						[&](DecaysOneOf<chef::re_empty, chef::re_char_class> auto const&) {
							backtrack();
						},
					},
					cur.cur->value);
			} while (!stack.empty());

			return false;
		}
	};
}

namespace chef {
	bool re_backtracking_engine::matches(chef::re const& re, std::string_view str)
	{
		re.to_string();
		::engine engine(re);
		return engine.matches(str);
	}
}
