#pragma once

#include <compare>
#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace chef {
	// A variable
	struct cfg_var {
		std::string value;

		friend bool operator==(cfg_var const&, cfg_var const&) = default;
		friend std::strong_ordering operator<=>(cfg_var const&, cfg_var const&) = default;
	};

	// One of the pre-lexed tokens
	struct cfg_token {
		int token_type;

		friend bool operator==(cfg_token const&, cfg_token const&) = default;
		friend std::strong_ordering operator<=>(cfg_token const&, cfg_token const&) = default;
	};

	// A straight sequence of variables or tokens, forms the parts of an alternative
	struct cfg_seq {
		using value_type = std::variant<cfg_var, cfg_token>;
		std::vector<value_type> value;

		friend bool operator==(cfg_seq const&, cfg_seq const&) = default;
		friend std::strong_ordering operator<=>(cfg_seq const&, cfg_seq const&) = default;
	};

	struct cfg_rule_body {
		std::vector<cfg_seq> alts;
	};

	struct cfg_rule {
		cfg_var var; // The variable this rule defines
		cfg_rule_body body;
	};

	class cfg {
	private:
		std::map<cfg_var, cfg_rule_body> rules_;

	public:
		cfg() = default;

		template <std::size_t N>
		explicit cfg(cfg_rule(&&rules)[N])
		{
			for (std::size_t i = 0; i < N; ++i) {
				add_rule(std::move(rules[i]));
			}
		}

		void add_rule(cfg_rule rule);
	};
}
