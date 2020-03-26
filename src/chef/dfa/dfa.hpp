#pragma once

#include <cassert>
#include <numeric>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <chef/_/bit_indices.hpp>
#include <chef/_/filter.hpp>
#include <chef/_/fold.hpp>
#include <chef/_/function_objects.hpp>
#include <chef/_/indexed.hpp>
#include <chef/_/irange.hpp>
#include <chef/_/lambda.hpp>
#include <chef/_/reserve_as.hpp>
#include <chef/_/transform.hpp>
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

    class dfa2 {
    public:
        using state_type = nfa::state_type;
        using symbol_type = nfa::symbol_type;

        explicit dfa2(std::size_t const num_states, std::vector<state_type> transition_table_data,
            state_type const start_state)
            : transition_table_data_(std::move(transition_table_data))
            , start_state_(std::move(start_state))
            , num_states_(num_states)
        {
            assert(transition_table_data_.size() % num_states == 0);
        }

        auto num_states() const -> std::size_t { return num_states_; }

        auto num_symbols() const -> std::size_t
        {
            return transition_table_data_.size() / num_states();
        }

        auto start_state() const -> state_type { return start_state_; }

        auto compute_next(state_type const current, symbol_type const symbol) const -> state_type
        {
            return mdview()[{current, symbol}];
        }

    private:
        std::vector<state_type> transition_table_data_;
        state_type start_state_;
        std::size_t num_states_;

        auto mdview() const
            -> decltype(_make_mdview<2>(transition_table_data_, num_states(), num_symbols()))
        {
            return _make_mdview<2>(transition_table_data_, num_states(), num_symbols());
        }
    };

    inline auto powerset_construction(nfa const& nfa) -> dfa
    {
        auto const original_states = nfa.states();

        assert(original_states.size() <= 32 && "Currently, max NFA supported size is 64 states");

        auto const num_symbols = nfa.num_symbols();
        auto states = std::vector<dfa::state_type>{(1u << nfa.start_state())};
        auto state_set = std::unordered_set<dfa::state_type>{states[0]};
        auto cur_index = std::size_t{0};

        auto transitions = std::unordered_map<dfa::state_type, std::vector<dfa::state_type>>{};

        do {
            auto const cur = states[cur_index++];

            static constexpr auto combine_multistates = [](auto&& states) {
                return std::accumulate(
                    states.begin(), states.iend(), dfa::state_type{0}, std::bit_or{});
            };

            static constexpr auto to_multistate = [](auto&& states) {
                auto state_masks
                    = chef::_transform(states, [](nfa::state_type state) { return 1u << state; });

                return combine_multistates(state_masks);
            };

            auto const compute_next_multistate = [cur, &nfa](auto const symbol) {
                auto bits = chef::_bit_indices(cur);

                auto state_mappings = chef::_transform(bits, [&](auto const state) {
                    return to_multistate(nfa.compute_next(state, symbol));
                });
                auto const next = combine_multistates(state_mappings);

                return next;
            };

            auto symbols = chef::_irange(dfa::symbol_type{0}, num_symbols);

            auto const& new_states
                = transitions
                      .emplace(std::piecewise_construct, std::forward_as_tuple(cur),
                          std::forward_as_tuple( //
                              chef::_fold(chef::_transform(symbols, compute_next_multistate),
                                  chef::_reserve_as(std::vector<dfa::state_type>(), num_symbols),
                                  chef::_by_value(CHEF_I_MEMFN_MUT(.push_back)))))
                      .first->second;

            states
                = chef::_fold(chef::_filter(new_states, [&] CHEF_I_L(state_set.insert(it).second)),
                    std::move(states), chef::_by_value(CHEF_I_MEMFN_MUT(.push_back)));
        } while (cur_index < states.size());

        auto state_mapping = std::unordered_map<dfa::state_type, dfa::state_type>{};
        state_mapping.reserve(states.size());

        for (auto const [index, value] : chef::_indexed<dfa::state_type>(states)) {
            state_mapping[value] = index;
        }

        auto const num_states = states.size();
        auto transition_table = std::vector<dfa::state_type>();
        transition_table.reserve(num_states * num_symbols);

        for (auto const state : states) {
            for (auto const symbol : chef::_irange(0, num_symbols)) {
                auto const dst = transitions[state][symbol];

                transition_table.push_back(state_mapping.at(dst));
            }
        }

        auto final_states = std::vector<dfa::state_type>();

        for (auto const state : states) {
            auto bits = chef::_bit_indices(state);
            auto is_final = std::find_if(bits.begin(), bits.iend(), [&nfa](auto const bit) {
                return nfa.is_final(bit);
            }) != bits.end();

            if (is_final) final_states.push_back(state);
        }

        return chef::dfa(states, transition_table, 0, std::move(final_states));
    }
}
