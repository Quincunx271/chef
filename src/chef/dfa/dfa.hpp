#pragma once

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>

#include <chef/dfa/nfa.hpp>

namespace chef {
    class dfa {
    public:
        using state_type = std::set<std::string>;
        using event_type = char;

        dfa() = default;

    private:
        using state_event_map_type = std::unordered_map<event_type, state_type>;

        std::map<state_type, state_event_map_type> transition_table_;
    };
}
