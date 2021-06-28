#include "./re.hpp"

#include <ostream>
#include <sstream>

#include <chef/_/overload.hpp>

using namespace std::literals;
using chef::detail::overload;

namespace chef {
	std::string re::to_string() const
	{
		return std::visit(overload{
							  [](chef::re_union const& re) {
								  std::ostringstream out;
								  bool first = true;

								  for (auto const& sub : re.pieces) {
									  if (!first) {
										  out << '|';
									  }
									  first = false;
									  out << sub->to_string();
								  }

								  return out.str();
							  },
							  [](chef::re_cat const& re) {
								  std::ostringstream out;

								  for (auto const& sub : re.pieces) {
									  bool const needs_parens
										  = std::holds_alternative<chef::re_union>(sub->value);
									  if (needs_parens) out << '(';
									  out << sub->to_string();
									  if (needs_parens) out << ')';
								  }

								  return out.str();
							  },
							  [](chef::re_star const& re) {
								  bool const needs_parens
									  = !std::holds_alternative<chef::re_lit>(re.value->value);
								  if (needs_parens) return '(' + re.value->to_string() + ")*";
								  return re.value->to_string() + '*';
							  },
							  [](chef::re_lit const& re) { return re.value; },
							  [](chef::re_empty const&) { return "<EMPTY>"s; },
							  [](chef::re_char_class const&) { return "[]"s; },
						  },
			value);
	}

	std::ostream& re::print_to(std::ostream& out) const
	{
		return out << to_string();
	}
}
