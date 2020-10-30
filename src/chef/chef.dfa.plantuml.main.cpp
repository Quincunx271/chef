#include <iostream>
#include <string_view>
#include <vector>

#include <chef/dfa/dfa.hpp>
#include <chef/dfa/plantuml.hpp>

using namespace std::literals;

/*

Run chef.dfa.plantuml, passing the NFA description into stdin.
Specify by the single commandline argument whether you wish the nfa or converted dfa to be printed.

This will output a UML state diagram in plantuml format, which can be piped into plantuml.

Sample usage:

printf '0 1 2\n1 0 1\n2 0 0\n2 1 1\n1 0 0' | chef.dfa.plantuml dfa \
    | java -jar plantuml.jar -pipe >dfa.png

*/

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "Usage: chef.dfa.plantuml [dfa|nfa]\n";
        return 1;
    }

    std::vector<std::tuple<unsigned int, unsigned int, unsigned int>> edge_list;

    unsigned int from, event, to;

    while (std::cin >> from >> event >> to) {
        edge_list.emplace_back(from, event, to);
    }

    auto nfa = chef::nfa(chef::nfa_builder(edge_list.begin(), edge_list.end()));

    if (argv[1] == "dfa"sv) {
        auto dfa = chef::powerset_construction(nfa);
        std::cout << chef::to_plantuml(dfa) << '\n';
    } else {
        std::cout << chef::to_plantuml(nfa) << '\n';
    }
}
