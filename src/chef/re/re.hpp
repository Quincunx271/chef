#pragma once

#include <algorithm>
#include <concepts>
#include <iosfwd>
#include <iterator>
#include <numeric>
#include <string>
#include <variant>
#include <vector>

#include <tl/tl.hpp>

#include <chef/_/concepts.hpp>
#include <chef/_/fwd.hpp>
#include <chef/_/overload.hpp>
#include <chef/_/value_ptr.hpp>

namespace chef {
	class re;

	struct re_union {
		std::vector<chef::detail::value_ptr<re>> pieces;
	};

	struct re_cat {
		std::vector<chef::detail::value_ptr<re>> pieces;
	};

	struct re_star {
		chef::detail::value_ptr<re> value;
	};

	struct re_lit {
		std::string value;
	};

	struct re_empty { };

	struct re_char_class { };

	class re {
	private:
		using variant_type
			= std::variant<re_empty, re_cat, re_union, re_lit, re_star, re_char_class>;

	public:
		variant_type value;

		re() = default; // Empty regular expression

		explicit re(variant_type value)
			: value(CHEF_MOVE(value))
		{ }

		explicit re(std::string lit)
			: value(re_lit(CHEF_MOVE(lit)))
		{ }

		friend re operator|(re lhs, re rhs)
		{
			re result = std::visit(
				detail::overload{
					[](re_union lhs, re_union rhs) -> re {
						lhs.pieces.insert(lhs.pieces.end(), std::move_iterator(rhs.pieces.begin()),
							std::move_iterator(rhs.pieces.end()));
						return re(CHEF_MOVE(lhs));
					},
					[](re_union lhs, detail::IsNot<re_empty> auto rhs) -> re {
						lhs.pieces.emplace_back(std::make_unique<re>(CHEF_MOVE(rhs)));
						return re(CHEF_MOVE(lhs));
					},
					[](detail::IsNot<re_empty> auto lhs, re_union rhs) -> re {
						rhs.pieces.emplace(
							rhs.pieces.begin(), std::make_unique<re>(CHEF_MOVE(lhs)));
						return re(CHEF_MOVE(rhs));
					},
					[](detail::IsNot<re_empty> auto lhs, detail::IsNot<re_empty> auto rhs) -> re {
						re_union result;
						result.pieces.emplace_back(std::make_unique<re>(CHEF_MOVE(lhs)));
						result.pieces.emplace_back(std::make_unique<re>(CHEF_MOVE(rhs)));
						return re(CHEF_MOVE(result));
					},
					[](re_empty, re_empty) -> re { return re(re_empty{}); },
					[](re_empty, auto rhs) -> re { return re(CHEF_MOVE(rhs)); },
					[](auto lhs, re_empty) -> re { return re(CHEF_MOVE(lhs)); },
				},
				CHEF_MOVE(lhs.value), CHEF_MOVE(rhs.value));

			return result;
		}

		friend re operator<<(re lhs, re rhs)
		{
			return std::visit(
				detail::overload{
					[](re_cat lhs, re_cat rhs) -> re {
						lhs.pieces.insert(lhs.pieces.end(), std::move_iterator(rhs.pieces.begin()),
							std::move_iterator(rhs.pieces.end()));
						return re(CHEF_MOVE(lhs));
					},
					[](re_cat lhs, detail::IsNot<re_empty> auto rhs) -> re {
						if_not_epsilon(rhs,
							[&] { lhs.pieces.emplace_back(std::make_unique<re>(CHEF_MOVE(rhs))); });
						return re(CHEF_MOVE(lhs));
					},
					[](detail::IsNot<re_empty> auto lhs, re_cat rhs) -> re {
						if_not_epsilon(lhs, [&] {
							rhs.pieces.emplace(
								rhs.pieces.begin(), std::make_unique<re>(CHEF_MOVE(lhs)));
						});
						return re(CHEF_MOVE(rhs));
					},
					[](detail::IsNot<re_empty> auto lhs, detail::IsNot<re_empty> auto rhs) -> re {
						re_cat result;
						if_not_epsilon(lhs, [&] {
							result.pieces.emplace_back(std::make_unique<re>(CHEF_MOVE(lhs)));
						});
						if_not_epsilon(rhs, [&] {
							result.pieces.emplace_back(std::make_unique<re>(CHEF_MOVE(rhs)));
						});
						if (result.pieces.size() == 1) {
							return re(CHEF_MOVE(*result.pieces.front()));
						} else if (result.pieces.empty()) {
							return re(re_lit(""));
						}
						return re(CHEF_MOVE(result));
					},
					[](re_lit lhs, re_lit rhs) -> re {
						return re(re_lit{CHEF_MOVE(lhs.value) + CHEF_MOVE(rhs.value)});
					},
					[](re_empty, auto&&) -> re { return re(re_empty{}); },
					[](auto&&, re_empty) -> re { return re(re_empty{}); },
					[](re_empty, re_empty) -> re { return re(re_empty{}); },
				},
				CHEF_MOVE(lhs.value), CHEF_MOVE(rhs.value));
		}

		re operator*() const&
		{
			if (std::holds_alternative<re_empty>(value)) return *this;
			if (std::holds_alternative<re_star>(value)) return *this;
			return re(re_star{chef::detail::value_ptr<re>(std::make_unique<re>(*this))});
		}

		re operator*() &&
		{
			if (std::holds_alternative<re_empty>(value)) return CHEF_MOVE(*this);
			if (std::holds_alternative<re_star>(value)) return CHEF_MOVE(*this);
			return re(re_star{chef::detail::value_ptr<re>(std::make_unique<re>(CHEF_MOVE(value)))});
		}

		// Whether the RE can be the empty string
		bool is_vanishable() const
		{
			return std::visit(detail::overload{
								  [](re_empty) { return false; },
								  [](re_star const&) { return true; },
								  [](re_lit const& lit) { return lit.value.empty(); },
								  [](re_cat const& cat) {
									  return std::all_of(cat.pieces.begin(), cat.pieces.end(),
										  [] TL(_1->is_vanishable()));
								  },
								  [](re_union const& alt) {
									  return std::any_of(alt.pieces.begin(), alt.pieces.end(),
										  [] TL(_1->is_vanishable()));
								  },
								  [](auto const&) { return false; },
							  },
				value);
		}

		friend std::ostream& operator<<(std::ostream& out, chef::re const& re)
		{
			return re.print_to(out);
		}

		std::string to_string() const;

		template <typename T, typename Acc>
		T accumulate_chars(T init, Acc&& acc) const
		{
			return std::visit(detail::overload{
								  [&](re_empty) { return CHEF_MOVE(init); },
								  [&](re_star const& re) {
									  return re.value->accumulate_chars(CHEF_MOVE(init), acc);
								  },
								  [&](re_lit const& lit) {
									  return std::accumulate(
										  lit.value.begin(), lit.value.end(), CHEF_MOVE(init), acc);
								  },
								  [&](re_cat const& cat) {
									  return std::accumulate(cat.pieces.begin(), cat.pieces.end(),
										  CHEF_MOVE(init), [&acc](T&& init, auto const& re_p) {
											  return re_p->accumulate_chars(CHEF_MOVE(init), acc);
										  });
								  },
								  [&](re_union const& alt) {
									  return std::accumulate(alt.pieces.begin(), alt.pieces.end(),
										  CHEF_MOVE(init), [&acc](T&& init, auto const& re_p) {
											  return re_p->accumulate_chars(CHEF_MOVE(init), acc);
										  });
								  },
								  [&](re_char_class) { return CHEF_MOVE(init); },
							  },
				value);
		}

	private:
		std::ostream& print_to(std::ostream&) const;

		template <typename R>
		static void if_not_epsilon(R const& re, auto&& fn)
		{
			if constexpr (std::same_as<chef::re_lit, R>) {
				if (!re.value.empty()) {
					fn();
				}
			} else {
				fn();
			}
		}
	};
}
