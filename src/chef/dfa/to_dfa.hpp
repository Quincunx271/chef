#pragma once

#include <bitset>
#include <type_traits>
#include <utility>

#include <chef/_/fwd.hpp>
#include <chef/dfa/dfa.hpp>
#include <chef/dfa/nfa.hpp>

namespace chef {
    template <typename StatesF, typename NumSymbolsF, typename StartStateF, typename ComputeNextF>
    struct nfa_behavior {
        [[no_unique_address]] StatesF states;
        [[no_unique_address]] NumSymbolsF num_symbols;
        [[no_unique_address]] StartStateF start_state;
        [[no_unique_address]] ComputeNextF compute_next;
    };

    template <typename StatesF, typename NumSymbolsF, typename StartStateF, typename ComputeNextF>
    nfa_behavior(StatesF, NumSymbolsF, StartStateF, ComputeNextF)
        ->nfa_behavior<StatesF, NumSymbolsF, StartStateF, ComputeNextF>;

    template <typename MState, typename FromMState>
    class _mstate_range {
        std::vector<MState> states_;
        [[no_unique_address]] FromMState from_mstate_;

    public:
        explicit constexpr _mstate_range(std::vector<MState> states, FromMState from_mstate)
            : states_(CHEF_I_MOVE(states))
            , from_mstate_(CHEF_I_MOVE(from_mstate))
        {}

        auto states() const -> auto const& /* Range<MState> */ { return states_; }

        auto decode(MState const& state) const -> decltype(auto) { return from_mstate_(state); }
    };

    constexpr auto _powset_const_impl
        = [](auto const& nfa, auto const& behavior, auto const& to_mstate,
              auto const& mstate_combiner,
              auto const& from_mstate) // -> std::pair<dfa2, StateVisitor>
    {
        using mstate_type = std::decay_t<decltype(to_mstate(behavior.start_state(nfa)))>;

        auto const original_states = behavior.states(nfa);

        auto const n_symbols = behavior.num_symbols(nfa);
        auto states = std::vector<mstate_type>({to_mstate(behavior.start_state(nfa))});
        auto state_set = std::unordered_set<mstate_type>({states[0]});
        auto cur_index = std::size_t{0};

        auto transitions = std::unordered_map<mstate_type, std::vector<mstate_type>>{};

        do {
            auto const cur = states[cur_index++];

            auto const combine_multistates = [&](auto&& states) {
                return std::accumulate(
                    states.begin(), states.iend(), mstate_type(), mstate_combiner);
            };

            auto const to_multistate = [&](auto&& states) {
                auto state_masks = chef::_transform(states, to_mstate);

                return combine_multistates(state_masks);
            };

            auto const compute_next_multistate = [&](auto const symbol) {
                auto bits = from_mstate(cur);

                auto state_mappings = chef::_transform(bits, [&](auto const state) {
                    return to_multistate(behavior.compute_next(nfa, state, symbol));
                });
                auto const next = combine_multistates(state_mappings);

                return next;
            };

            auto symbols = chef::_irange(dfa::symbol_type{0}, n_symbols);

            auto const& new_states
                = transitions
                      .emplace(std::piecewise_construct, std::forward_as_tuple(cur),
                          std::forward_as_tuple( //
                              chef::_fold(chef::_transform(symbols, compute_next_multistate),
                                  chef::_reserve_as(std::vector<mstate_type>(), n_symbols),
                                  chef::_by_value(CHEF_I_MEMFN_MUT(.push_back)))))
                      .first->second;

            states
                = chef::_fold(chef::_filter(new_states, [&] CHEF_I_L(state_set.insert(it).second)),
                    CHEF_I_MOVE(states), chef::_by_value(CHEF_I_MEMFN_MUT(.push_back)));
        } while (cur_index < states.size());

        auto state_mapping = std::unordered_map<mstate_type, dfa::state_type>{};
        state_mapping.reserve(states.size());

        for (auto const [index, value] : chef::_indexed<dfa::state_type>(states)) {
            state_mapping[value] = index;
        }

        auto const num_states = states.size();
        auto transition_table = std::vector<dfa::state_type>();
        transition_table.reserve(num_states * n_symbols);

        for (auto const state : states) {
            for (auto const symbol : chef::_irange(0, n_symbols)) {
                auto const dst = transitions[state][symbol];

                transition_table.push_back(state_mapping.at(dst));
            }
        }

        return std::pair(chef::dfa2(num_states, transition_table, 0),
            _mstate_range(CHEF_I_MOVE(states), from_mstate));
    };

    template <std::size_t SmallSize = 64>
    constexpr auto _powerset_construction2 = [](auto const& nfa, auto const& behavior) {
        using std::size;

        auto const original_states = behavior.states(nfa);

        assert(size(original_states) <= SmallSize
            && "NFA has too many states for current implementation");

        return _powset_const_impl(nfa, behavior,
            [](auto const nfa_state) {
                auto mstate = std::bitset<SmallSize>();
                mstate.set(nfa_state);
                return mstate;
            },
            std::bit_or(), chef::_bit_indices);
    };

    template <std::size_t SmallSize = 64>
    constexpr auto powerset_construction2 = chef::_overload(
        [](auto const& nfa, auto const& behavior) {
            return _powerset_construction2<SmallSize>(nfa, behavior);
        },
        [](auto const& nfa) {
            return _powerset_construction2<SmallSize>(nfa,
                chef::nfa_behavior{
                    .states = CHEF_I_MEMFN(.states),
                    .num_symbols = CHEF_I_MEMFN(.num_symbols),
                    .start_state = CHEF_I_MEMFN(.start_state),
                    .compute_next = CHEF_I_MEMFN(.compute_next),
                });
        });
}
