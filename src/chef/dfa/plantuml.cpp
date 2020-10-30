#include "./plantuml.hpp"

#include <ostream>

#include <chef/_/irange.hpp>
#include <chef/dfa/dfa.hpp>

namespace chef {
    template <>
    auto to_plantuml<chef::dfa>::do_write(std::ostream& out, to_plantuml<chef::dfa> const& value)
        -> std::ostream&
    {
        out << "@startuml\n";

        auto const& dfa = *value.fa;

        out << "[*] --> " << dfa.start_state() << '\n';
        for (auto state : chef::_irange(0, dfa.num_states())) {
            out << state << ":\n";

            for (auto symbol : chef::_irange(0, dfa.num_symbols())) {
                auto next = dfa.compute_next(state, symbol);
                out << state << " --> " << next << " : " << symbol << '\n';
            }
        }

        return out << "@enduml";
    }

    template <>
    auto to_plantuml<chef::nfa>::do_write(std::ostream& out, to_plantuml<chef::nfa> const& value)
        -> std::ostream&
    {
        out << "@startuml\n";

        auto const& nfa = *value.fa;

        out << "[*] --> " << nfa.start_state() << '\n';
        for (auto state : chef::_irange(0, nfa.num_states())) {
            out << state << ":\n";

            for (auto symbol : chef::_irange(0, nfa.num_symbols())) {
                auto next = nfa.compute_next(state, symbol);
                for (auto next_state : next) {
                    out << state << " --> " << next_state << " : " << symbol << '\n';
                }
            }
        }

        return out << "@enduml";
    }
}
