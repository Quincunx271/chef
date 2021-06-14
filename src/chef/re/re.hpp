#pragma once

#include <iterator>
#include <string>
#include <variant>
#include <vector>

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

	struct re_char_class { };

	class re {
	private:
		using variant_type = std::variant<re_cat, re_union, re_lit, re_star, re_char_class>;

	public:
		variant_type value;

		explicit re(variant_type value)
			: value(std::move(value))
		{ }

		explicit re(std::string lit)
			: value(re_lit(std::move(lit)))
		{ }

		friend re operator|(re lhs, re rhs)
		{
			re_union result = std::visit(
				detail::overload{
					[](re_union lhs, re_union rhs) -> re_union {
						lhs.pieces.insert(lhs.pieces.end(), std::move_iterator(rhs.pieces.begin()),
							std::move_iterator(rhs.pieces.end()));
						return lhs;
					},
					[](re_union lhs, auto rhs) -> re_union {
						lhs.pieces.emplace_back(std::make_unique<re>(std::move(rhs)));
						return lhs;
					},
					[](auto lhs, re_union rhs) -> re_union {
						rhs.pieces.emplace(
							rhs.pieces.begin(), std::make_unique<re>(std::move(lhs)));
						return rhs;
					},
					[](auto lhs, auto rhs) -> re_union {
						re_union result;
						result.pieces.emplace_back(std::make_unique<re>(std::move(lhs)));
						result.pieces.emplace_back(std::make_unique<re>(std::move(rhs)));
						return result;
					},
				},
				std::move(lhs.value), std::move(rhs.value));

			return re(std::move(result));
		}

		friend re operator<<(re lhs, re rhs)
		{
			variant_type result = std::visit(
				detail::overload{
					[](re_cat lhs, re_cat rhs) -> variant_type {
						lhs.pieces.insert(lhs.pieces.end(), std::move_iterator(rhs.pieces.begin()),
							std::move_iterator(rhs.pieces.end()));
						return lhs;
					},
					[](re_cat lhs, auto rhs) -> variant_type {
						lhs.pieces.emplace_back(std::make_unique<re>(std::move(rhs)));
						return lhs;
					},
					[](auto lhs, re_cat rhs) -> variant_type {
						rhs.pieces.emplace(
							rhs.pieces.begin(), std::make_unique<re>(std::move(lhs)));
						return rhs;
					},
					[](auto lhs, auto rhs) -> variant_type {
						re_cat result;
						result.pieces.emplace_back(std::make_unique<re>(std::move(lhs)));
						result.pieces.emplace_back(std::make_unique<re>(std::move(rhs)));
						return result;
					},
					[](re_lit lhs, re_lit rhs) -> variant_type {
						return re_lit{std::move(lhs.value) + std::move(rhs.value)};
					},
				},
				std::move(lhs.value), std::move(rhs.value));

			return re(std::move(result));
		}

		re operator*() const&
		{
			if (std::holds_alternative<re_star>(value)) return *this;
			return re(re_star{chef::detail::value_ptr<re>(std::make_unique<re>(*this))});
		}

		re operator*() &&
		{
			if (std::holds_alternative<re_star>(value)) return std::move(*this);
			return re(re_star{chef::detail::value_ptr<re>(std::make_unique<re>(std::move(value)))});
		}
	};
}
