#pragma once

#include <vector>

#include <chef/dfa/nfa.hpp>

namespace chef {
    class dfa {
    public:
        using state_type = nfa::state_type;
        using symbol_type = nfa::symbol_type;

        template <typename States, typename FinalSet>
        explicit dfa(States const& states, std::vector<state_type> transition_table_data,
            state_type const start_state, FinalSet const& final_states)
            : transition_table_data_(std::move(transition_table_data))
            , start_state_(std::move(start_state))
            , final_states_set_(states.size())
        {
            assert(transition_table_data_.size() % states.size() == 0);

            for (state_type const f : final_states) {
                final_states_set_[f] = true;
            }
        }

        auto num_states() const -> std::size_t { return final_states_set_.size(); }

        auto num_symbols() const -> std::size_t
        {
            return transition_table_data_.size() / num_states();
        }

        auto start_state() const -> state_type { return start_state_; }

        auto final_states() const -> std::vector<state_type>
        {
            auto result = std::vector<state_type>();

            for (auto st = state_type(0); st < final_states_set_.size(); ++st) {
                if (final_states_set_[st]) result.push_back(st);
            }

            return result;
        }

        bool is_final_state(state_type const state) const
        {
            assert(state < num_states());
            return final_states_set_[state];
        }

        auto compute_next(state_type const current, symbol_type const symbol) const -> state_type
        {
            return mdview()[{current, symbol}];
        }

    private:
        std::vector<state_type> transition_table_data_;
        state_type start_state_;
        std::vector<bool> final_states_set_;

        auto mdview() const
            -> decltype(_make_mdview<2>(transition_table_data_, num_states(), num_symbols()))
        {
            return _make_mdview<2>(transition_table_data_, num_states(), num_symbols());
        }
    };

    inline auto powerset_construction(nfa const& nfa) -> dfa { auto const states = nfa.states(); }
}
