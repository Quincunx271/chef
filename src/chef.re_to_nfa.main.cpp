#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

#include <chef/re/re.hpp>
#include <chef/re/to_nfa.hpp>

using namespace std::literals;

/*

Run chef.re_to_nfa, passing the RE description into stdin.
This will output an NFA capable of being consumed by chef.dfa.plantuml.

*/

namespace {
	chef::re parse(std::string_view str)
	{
		if (str.empty()) return chef::re(chef::re_lit{""});

		switch (str.front()) {
            case '.':
		case '[': std::cerr << "Character classes are currently unsupported\n"; std::exit(1);
        case '(': // TODO: parse parenthesized
        case '.'
		}
	}
}

int main(int argc, char** argv)
{
	std::string str;
	if (!std::getline(std::cin, str)) return 1;
	const chef::re re = parse(str);

	auto nfa_result = chef::to_nfa(re);

	std::cout << "Symbol mapping:\n";
	for (auto [c, i] : nfa_result.symbol_map) {
		std::cout << "\t" << (char)c << " --> " << static_cast<std::size_t>(i) << ",\n";
	}
	std::cout << '\n';

	std::cout << "Final states:";
	char sep = ' ';
	for (auto accept : nfa_result.accepts) {
		std::cout << sep << accept;
		sep = ',';
	}
	std::cout << "\n\n";

	std::cout << "Transition table:\n";

	for (const chef::state_type from : nfa_result.nfa.states()) {
		for (const chef::symbol_type sym : nfa_result.nfa.symbols()) {
			for (const chef::state_type to : nfa_result.nfa.process(from, sym)) {
				std::cout << std::size_t(from) << ' ' << std::size_t(to) << ' ' << std::size_t(to)
						  << ' ';
			}
		}
	}
	std::cout << '\n';
}
