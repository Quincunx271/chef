#pragma once

#include <initializer_list>
#include <ostream>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <variant>

#include <cstdint>
#include <numeric>
#include <set>
#include <vector>

#include <chef/_function_objects.hpp>
#include <chef/_indexed.hpp>
#include <chef/_mdview.hpp>
#include <chef/_span.hpp>

namespace chef {
    class nfa_builder {
    public:
        using state_type = std::uint32_t;
        using symbol_type = std::uint32_t;

        template <typename FwdIter>
        explicit nfa_builder(FwdIter begin, FwdIter end)
        {
            constexpr auto from_index = 0;
            constexpr auto symbol_index = 1;
            constexpr auto to_index = 2;

            auto const num_symbols
                = std::transform_reduce(begin, end, symbol_type(0), _max, _get_index<symbol_index>)
                + 1;

            auto const num_states = std::transform_reduce(begin, end,
                                        std::transform_reduce(begin, end, state_type(0), _max,
                                            _get_index<from_index>),
                                        _max, _get_index<to_index>)
                + 1;

            transition_table_.assign(static_cast<std::size_t>(num_states),
                std::vector<std::vector<state_type>>(num_symbols));

            while (begin != end) {
                auto const [from, symbol, to] = *begin;

                transition_table_[from][symbol].push_back(to);

                ++begin;
            }
        }

        explicit nfa_builder(
            std::initializer_list<std::tuple<state_type, symbol_type, state_type>> data)
            : nfa_builder{data.begin(), data.end()}
        {}

        auto const& compute_next(state_type state, symbol_type symbol) const
        {
            return transition_table_.at(state).at(symbol);
        }

        auto states() const -> std::vector<state_type>
        {
            std::vector<state_type> result;
            result.resize(transition_table_.size());
            std::iota(begin(result), end(result), 0);

            return result;
        }

        auto num_states() const noexcept -> state_type { return transition_table_.size(); }

        auto num_symbols() const noexcept -> symbol_type
        {
            return transition_table_.front().size();
        }

        auto start_state() const -> state_type { return start_state_; }

        auto final_states() const -> std::vector<state_type> { return final_states_; }

        friend auto operator<<(std::ostream& out, nfa_builder const& nfa) -> std::ostream&
        {
            out << "nfa:\n";

            for (auto&& [from, symbol_table] : _indexed<state_type>(nfa.transition_table_)) {
                for (auto&& [symbol, dst] : _indexed<symbol_type>(symbol_table)) {
                    out << from << " + " << symbol << " -> {";

                    for (auto const it : dst) {
                        out << it << ", ";
                    }

                    out << "}\n";
                }

                out << '\n';
            }

            return out;
        }

    private:
        // current state, next symbol, {next states}
        std::vector<std::vector<std::vector<state_type>>> transition_table_;

        state_type start_state_ = {};
        std::vector<state_type> final_states_ = {};
    };

    class nfa {
    public:
        using state_type = std::uint32_t;
        using symbol_type = std::uint32_t;

        explicit /*constexpr*/ nfa(nfa_builder const& builder)
            : num_states_(builder.num_states())
            , num_symbols_(builder.num_symbols())
            , start_state_(builder.start_state())
            , final_states_(builder.final_states())
            , transition_table_data_()
        {
            auto const data_size = [&] {
                std::size_t acc = 0;

                for (state_type from = 0; from < num_states_; ++from) {
                    for (symbol_type symbol = 0; symbol < num_symbols_; ++symbol) {
                        acc += builder.compute_next(from, symbol).size();
                    }
                }

                return acc;
            }();

            transition_table_.reserve(num_states_ * num_symbols_);
            transition_table_data_.reserve(data_size);

            for (state_type from = 0; from < num_states_; ++from) {
                for (symbol_type symbol = 0; symbol < num_symbols_; ++symbol) {
                    auto const& next = builder.compute_next(from, symbol);

                    auto location = transition_table_data_.insert(
                        transition_table_data_.end(), next.begin(), next.end());

                    transition_table_.push_back(chef::_span<state_type const>(
                        transition_table_data_.data() + (location - transition_table_data_.begin()),
                        next.size()));
                }
            }

            std::sort(final_states_.begin(), final_states_.end());
        }

        auto compute_next(state_type state, symbol_type symbol) const
            -> chef::_span<state_type const>
        {
            return mdview().at(symbol, state);
        }

        auto num_states() const -> std::size_t { return num_states_; }

        auto num_symbols() const -> std::size_t { return num_symbols_; }

        auto states() const -> std::vector<state_type>
        {
            std::vector<state_type> result;
            result.resize(num_states());
            std::iota(begin(result), end(result), 0);

            return result;
        }

        auto start_state() const -> state_type { return start_state_; }

        auto final_states() const -> std::vector<state_type> { return final_states_; }

        friend auto operator<<(std::ostream& out, nfa const& nfa) -> std::ostream&
        {
            out << "nfa:\n";

            auto const tbl = nfa.mdview();

            for (state_type from = 0; from < nfa.num_states_; ++from) {
                for (symbol_type symbol = 0; symbol < nfa.num_symbols_; ++symbol) {
                    out << from << " + " << symbol << " -> {";

                    for (auto const to : tbl[{symbol, from}]) {
                        out << to << ", ";
                    }

                    out << "}\n";
                }

                out << '\n';
            }

            return out;
        }

    private:
        // current state, next symbol, {next states}
        std::vector<chef::_span<state_type const>> transition_table_;

        state_type num_states_ = {};
        symbol_type num_symbols_ = {};

        state_type start_state_ = {};
        std::vector<state_type> final_states_ = {};

        std::vector<state_type> transition_table_data_;

        auto mdview() const
            -> decltype(_make_mdview<2>(transition_table_, num_symbols_, num_states_))
        {
            return _make_mdview<2>(transition_table_, num_symbols_, num_states_);
        }
    };
}
