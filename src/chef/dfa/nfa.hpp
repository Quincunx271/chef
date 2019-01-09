#pragma once

#include <initializer_list>
#include <ostream>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <chef/dfa/epsilon.hpp>

namespace chef {
    class dfa;

    class nfa
    {
        friend class dfa;

    public:
        using state_type = std::string;
        using event_type = std::variant<char, epsilon_t>;

        nfa() = default;

        template <typename Iter>
        explicit nfa(Iter begin, Iter end)
        {
            while (begin != end) {
                transition_table_[std::get<0>(*begin)][std::get<1>(*begin)]
                    .insert(std::get<2>(*begin));

                ++begin;
            }

            for (auto&& [from, event_table] : transition_table_) {
                compute_eps(
                    event_table[chef::epsilon], from, transition_table_);
            }
        }

    private:
        template <typename Tbl>
        static void compute_eps(
            std::set<state_type>& result, state_type const& cur, Tbl&& tbl)
        {
            auto&& it = tbl[cur][chef::epsilon];

            std::for_each(it.begin(), it.end(), [&](state_type const& state) {
                if (cur != state) {
                    compute_eps(result, state, tbl);
                }
            });

            result.insert(cur);
        }

    public:
        explicit nfa(std::initializer_list<
            std::tuple<state_type, event_type, state_type>>
                data)
            : nfa{data.begin(), data.end()}
        {}

        auto const& operator[](state_type state) const
        {
            return transition_table_.at(state);
        }

        std::vector<state_type> states() const
        {
            std::vector<state_type> result;
            result.reserve(transition_table_.size());

            for (auto&& [key, ig] : transition_table_) {
                result.push_back(key);
            }

            return result;
        }

        friend std::ostream& operator<<(std::ostream& out, nfa const& nfa)
        {
            out << "nfa:\n";

            for (auto&& [from, event_table] : nfa.transition_table_) {
                for (auto&& [event, dst] : event_table) {
                    out << from << " + ";
                    std::visit(
                        [&out](auto it) {
                            if constexpr (std::is_same_v<decltype(it),
                                              epsilon_t>) {
                                out << "eps";
                            } else {
                                out << it;
                            }
                        },
                        event);
                    out << " -> ";

                    for (auto&& it : dst) {
                        out << it << ", ";
                    }

                    out << '\n';
                }

                out << '\n';
            }

            return out;
        }

    private:
        using state_event_map_type
            = std::unordered_map<event_type, std::set<state_type>>;

        state_type starting_state_;
        std::set<state_type> accepting_states_;

        std::unordered_map<state_type, state_event_map_type> transition_table_;
    };
}
