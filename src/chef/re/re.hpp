#pragma once

#include <algorithm>
#include <concepts>
#include <iosfwd>
#include <iterator>
#include <string>
#include <variant>
#include <vector>

#include <tl/tl.hpp>

#include <chef/_/concepts.hpp>
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
			: value(std::move(value))
		{ }

		explicit re(std::string lit)
			: value(re_lit(std::move(lit)))
		{ }

		friend re operator|(re lhs, re rhs)
		{
			re result = std::visit(
				detail::overload{
					[](re_union lhs, re_union rhs) -> re {
						lhs.pieces.insert(lhs.pieces.end(), std::move_iterator(rhs.pieces.begin()),
							std::move_iterator(rhs.pieces.end()));
						return re(std::move(lhs));
					},
					[](re_union lhs, detail::IsNot<re_empty> auto rhs) -> re {
						lhs.pieces.emplace_back(std::make_unique<re>(std::move(rhs)));
						return re(std::move(lhs));
					},
					[](detail::IsNot<re_empty> auto lhs, re_union rhs) -> re {
						rhs.pieces.emplace(
							rhs.pieces.begin(), std::make_unique<re>(std::move(lhs)));
						return re(std::move(rhs));
					},
					[](detail::IsNot<re_empty> auto lhs, detail::IsNot<re_empty> auto rhs) -> re {
						re_union result;
						result.pieces.emplace_back(std::make_unique<re>(std::move(lhs)));
						result.pieces.emplace_back(std::make_unique<re>(std::move(rhs)));
						return re(std::move(result));
					},
					[](re_empty, re_empty) -> re { return re(re_empty{}); },
					[](re_empty, auto rhs) -> re { return re(std::move(rhs)); },
					[](auto lhs, re_empty) -> re { return re(std::move(lhs)); },
				},
				std::move(lhs.value), std::move(rhs.value));

			return result;
		}

		friend re operator<<(re lhs, re rhs)
		{
			return std::visit(
				detail::overload{
					[](re_cat lhs, re_cat rhs) -> re {
						lhs.pieces.insert(lhs.pieces.end(), std::move_iterator(rhs.pieces.begin()),
							std::move_iterator(rhs.pieces.end()));
						return re(std::move(lhs));
					},
					[](re_cat lhs, detail::IsNot<re_empty> auto rhs) -> re {
						if_not_epsilon(rhs,
							[&] { lhs.pieces.emplace_back(std::make_unique<re>(std::move(rhs))); });
						return re(std::move(lhs));
					},
					[](detail::IsNot<re_empty> auto lhs, re_cat rhs) -> re {
						if_not_epsilon(lhs, [&] {
							rhs.pieces.emplace(
								rhs.pieces.begin(), std::make_unique<re>(std::move(lhs)));
						});
						return re(std::move(rhs));
					},
					[](detail::IsNot<re_empty> auto lhs, detail::IsNot<re_empty> auto rhs) -> re {
						re_cat result;
						if_not_epsilon(lhs, [&] {
							result.pieces.emplace_back(std::make_unique<re>(std::move(lhs)));
						});
						if_not_epsilon(rhs, [&] {
							result.pieces.emplace_back(std::make_unique<re>(std::move(rhs)));
						});
						if (result.pieces.size() == 1) {
							return re(std::move(*result.pieces.front()));
						} else if (result.pieces.empty()) {
							return re(re_lit(""));
						}
						return re(std::move(result));
					},
					[](re_lit lhs, re_lit rhs) -> re {
						return re(re_lit{std::move(lhs.value) + std::move(rhs.value)});
					},
					[](re_empty, auto&&) -> re { return re(re_empty{}); },
					[](auto&&, re_empty) -> re { return re(re_empty{}); },
					[](re_empty, re_empty) -> re { return re(re_empty{}); },
				},
				std::move(lhs.value), std::move(rhs.value));
		}

		re operator*() const&
		{
			if (std::holds_alternative<re_empty>(value)) return *this;
			if (std::holds_alternative<re_star>(value)) return *this;
			return re(re_star{chef::detail::value_ptr<re>(std::make_unique<re>(*this))});
		}

		re operator*() &&
		{
			if (std::holds_alternative<re_empty>(value)) return std::move(*this);
			if (std::holds_alternative<re_star>(value)) return std::move(*this);
			return re(re_star{chef::detail::value_ptr<re>(std::make_unique<re>(std::move(value)))});
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
