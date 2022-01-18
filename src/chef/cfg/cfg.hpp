#pragma once

#include <cassert>
#include <compare>
#include <iosfwd>
#include <map>
#include <set>
#include <span>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <chef/_/fwd.hpp>
#include <chef/util/ranges.hpp>

namespace chef {
	struct cfg_var;
	struct cfg_token;
	struct cfg_seq;
	struct cfg_rule;

	namespace detail {
		std::ostream& print_to(std::ostream& out, const cfg_var&);
		std::ostream& print_to(std::ostream& out, const cfg_token&);
		std::ostream& print_to(std::ostream& out, const cfg_seq&);
		std::ostream& print_to(std::ostream& out, const cfg_rule&);
	}

	// A variable
	struct cfg_var {
		std::string value;

		friend bool operator==(cfg_var const&, cfg_var const&) = default;
		friend std::strong_ordering operator<=>(cfg_var const&, cfg_var const&) = default;

		friend std::ostream& operator<<(std::ostream& out, chef::cfg_var const& var)
		{
			return detail::print_to(out, var);
		}
	};

	// One of the pre-lexed tokens
	struct cfg_token {
		int token_type;

		friend constexpr bool operator==(cfg_token const&, cfg_token const&) = default;
		friend constexpr std::strong_ordering operator<=>(cfg_token const&, cfg_token const&)
			= default;

		friend std::ostream& operator<<(std::ostream& out, chef::cfg_token const& token)
		{
			return detail::print_to(out, token);
		}
	};

	inline namespace literals {
		inline cfg_var operator""_var(const char* str, std::size_t len)
		{
			return cfg_var(std::string(str, len));
		}

		inline cfg_token operator""_tok(unsigned long long value)
		{
			return cfg_token((int)value);
		}
	}

	inline constexpr cfg_token cfg_epsilon{-1};
	inline constexpr cfg_token cfg_eof{-2};

	// A straight sequence of variables or tokens, forms the parts of an alternative
	struct cfg_seq {
		using value_type = std::variant<cfg_var, cfg_token>;
		std::vector<value_type> value;

		friend bool operator==(cfg_seq const&, cfg_seq const&) = default;
		friend std::strong_ordering operator<=>(cfg_seq const&, cfg_seq const&) = default;

		auto begin() const
		{
			return value.begin();
		}

		auto end() const
		{
			return value.end();
		}

		auto rbegin() const
		{
			return value.rbegin();
		}

		auto rend() const
		{
			return value.rend();
		}

		friend std::ostream& operator<<(std::ostream& out, chef::cfg_seq const& seq)
		{
			return detail::print_to(out, seq);
		}
	};

	static_assert(std::ranges::forward_range<cfg_seq>);

	struct cfg_rule_body {
		std::vector<cfg_seq> alts;
	};

	struct cfg_rule {
		cfg_var var; // The variable this rule defines
		cfg_rule_body body;

		friend std::ostream& operator<<(std::ostream& out, chef::cfg_rule const& rule)
		{
			return detail::print_to(out, rule);
		}
	};

	class cfg {
	private:
		cfg_var start_;
		std::map<cfg_var, cfg_rule_body> rules_;

	public:
		explicit cfg(cfg_var start)
			: start_(CHEF_MOVE(start))
		{ }

		template <std::size_t N>
		explicit cfg(cfg_var start, cfg_rule(&&rules)[N])
			: cfg(CHEF_MOVE(start))
		{
			for (std::size_t i = 0; i < N; ++i) {
				add_rule(std::move(rules[i]));
			}
		}

		void add_rule(cfg_rule rule);

		const cfg_var& start_var() const
		{
			return start_;
		}

		ForwardRangeOf<const cfg_var&> auto vars() const;

		ForwardRangeOf<const cfg_seq&> auto rules(cfg_var const&) const;
	};

	std::map<cfg_var, std::set<cfg_token>> first_sets(const cfg&);

	std::map<cfg_var, std::set<cfg_token>> follow_sets(
		const cfg&, const std::map<cfg_var, std::set<cfg_token>>& first_sets);

	std::set<cfg_token> first_plus_set(const cfg_var& var, const cfg_seq& rule,
		const std::map<cfg_var, std::set<cfg_token>>& first_sets,
		const std::map<cfg_var, std::set<cfg_token>>& follow_sets);

	inline ForwardRangeOf<const cfg_var&> auto cfg::vars() const
	{
		return std::ranges::views::keys(rules_);
	}

	inline ForwardRangeOf<const cfg_seq&> auto cfg::rules(cfg_var const& var) const
	{
		auto it = rules_.find(var);
		assert(it != rules_.end());
		return std::span(std::as_const(it->second.alts));
	}
}
