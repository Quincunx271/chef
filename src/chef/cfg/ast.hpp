#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include <chef/cfg/cfg.hpp>

// Defines a runtime AST for chef.
//
// It remains to be decided whether this will include concrete pieces such as
// punctuation, or whether it will only include the abstract information.

namespace chef::rt_ast {
	struct ast_node {
		using element_type = std::variant<cfg_token, std::unique_ptr<ast_node>>;

		// The name of the rule that produced this node.
		std::string name;
		// For nodes which are not the full node of the rule, this identifies
		// which piece of the larger rule the node comes from.
		std::size_t id = 0;

		std::vector<element_type> children;
	};
}
