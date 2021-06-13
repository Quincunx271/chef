#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <chef/_/fwd.hpp>
#include <chef/_/ranges.hpp>
#include <chef/dfa/convert.hpp>
#include <chef/dfa/dfa.hpp>
#include <chef/dfa/nfa.hpp>
#include <chef/dfa/plantuml.hpp>

using namespace std::literals;

/*

Run chef.dfa.plantuml, passing the NFA description into stdin.
Specify by the single commandline argument whether you wish the nfa or converted dfa to be printed.

This will output a UML state diagram in plantuml format, which can be piped into plantuml.

Sample usage:

printf '0 1 2\n1 0 1\n2 0 0\n2 1 1\n1 0 0' | chef.dfa.plantuml dfa \
    | java -jar plantuml.jar -pipe >dfa.png

printf '0 1 2\n1 0 1\n2 0 0\n2 1 1\n1 0 0\n0 2 0\n' | chef.dfa.plantuml --minimize --final=2 dfa |
java -jar plantuml.jar -pipe >dfa.png
*/

void print_usage()
{
	std::cerr << "Usage: chef.dfa.plantuml [--minimize] ['--final=1,4,2'] [dfa|nfa]\n";
}

int main(int argc, char** argv)
{
	std::unordered_set<chef::state_type> final_states;
	bool is_dfa = false;
	bool minimize = false;
	if (argc != 2) {
		if (argc == 4 && argv[1] == "--minimize"sv
			&& std::string_view(argv[2]).starts_with("--final=")) {
			minimize = true;
			is_dfa = true;
			if (argv[3] != "dfa"sv) {
				std::cerr << "Only DFAs are minimizable\n";
				return 1;
			}
			std::string_view finals = argv[2];
			finals.remove_prefix("--final="sv.size());

			for (auto&& section : std::ranges::views::split(finals, ',')) {
				auto com_section = std::ranges::views::common(CHEF_FWD(section));
				final_states.insert(std::stoi(std::string(com_section.begin(), com_section.end())));
			}
		} else {
			print_usage();
			return 1;
		}
	} else {
		if (argv[1] != "dfa"sv && argv[1] != "nfa"sv) {
			print_usage();
			return 1;
		}
		is_dfa = argv[1] == "dfa"sv;
	}

	std::vector<chef::fa_edge> edge_list;
	std::unordered_set<unsigned int> states;
	std::unordered_set<unsigned int> symbols;

	unsigned int from, symbol, to;

	while (std::cin >> from >> to >> symbol) {
		edge_list.emplace_back(from, to, symbol);
		states.insert(from);
		states.insert(to);
		symbols.insert(symbol);
	}

	auto nfa = chef::nfa(static_cast<chef::state_type>(states.size()),
		static_cast<chef::symbol_type>(symbols.size()), edge_list);

	if (is_dfa) {
		auto [dfa, categories] = chef::to_dfa(nfa, {final_states});
		if (minimize) {
			std::cerr << "DFA minimization currently unsupported\n";
		}

		std::unordered_map<chef::state_type, std::string> labels;
		assert(categories.size() == 1);
		for (chef::state_type const final_state : categories[0]) {
			labels.emplace(final_state, "final"s);
		}

		std::cout << chef::to_plantuml(dfa, labels) << '\n';
	} else {
		std::cout << chef::to_plantuml(nfa) << '\n';
	}
}
