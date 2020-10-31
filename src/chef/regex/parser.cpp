#include "./parser.hpp"

#include <cassert>
#include <optional>
#include <vector>

using namespace std::literals;

namespace chef {
    auto parse_ll1_re(std::string_view regex, char skip = '\0')
        -> std::optional<std::pair<chef::_regex::regex, std::string_view>>;

    auto parse_ll1_paren(std::string_view regex)
        -> std::optional<std::pair<chef::_regex::regex, std::string_view>>
    {
        if (regex[0] != '(') return std::nullopt;
        regex.remove_prefix(1);
        auto r = parse_ll1_re(regex);

        if (!r) return std::nullopt;
        if (r->second.empty()) return std::nullopt;
        if (r->second[0] != ')') return std::nullopt;

        r->second.remove_prefix(1);

        return r;
    }

    auto parse_ll1_altp(chef::_regex::regex first, std::string_view regex)
        -> std::optional<std::pair<chef::_regex::regex, std::string_view>>
    {
        if (regex.empty() || regex[0] != '|') return std::nullopt;

        auto alt = chef::_regex::alternative {};
        do {
            regex.remove_prefix(1);
            auto next = parse_ll1_re(regex, /* skip= */ '|');
            if (!next) return std::nullopt;
            alt.alternatives.push_back(std::make_unique<chef::_regex::regex>(
                chef::_regex::regex {std::move(next->first)}));
            regex = next->second;
        } while (!regex.empty() && regex[0] == '|');

        return std::optional(std::pair({std::move(alt)}, regex));
    }

    auto parse_ll1_seqp(chef::_regex::regex first, std::string_view regex)
        -> std::optional<std::pair<chef::_regex::regex, std::string_view>>
    {
        if (regex.empty()) return std::optional(std::pair(std::move(first), ""sv));

        auto seq = chef::_regex::sequence {};
        do {
            auto next = parse_ll1_re(regex);
            if (!next) return std::nullopt;
            regex = next->second;
            if (regex[0] == '*') {
                regex.remove_prefix(1);
                next = std::optional(
                    std::pair(chef::_regex::regex {chef::_regex::kleene_star {
                                  std::make_unique<chef::_regex::regex>(
                                      chef::_regex::regex {std::move(next->first)})}},
                        regex));
            }

            seq.sequence.push_back(std::make_unique<chef::_regex::regex>(
                chef::_regex::regex {std::move(next->first)}));
        } while (!regex.empty() && regex[0] != '|' && regex[0] != ')');

        return std::optional(std::pair(chef::_regex::regex {std::move(seq)}, regex));
    }

    auto parse_ll1_re(std::string_view regex, char skip)
        -> std::optional<std::pair<chef::_regex::regex, std::string_view>>
    {
        if (regex.empty()) return std::optional(std::pair({""s}, ""sv));
        if (regex[0] == '*') return std::nullopt;

        auto first = chef::_regex::regex {};

        if (regex[0] == '(') {
            auto p = parse_ll1_paren(regex);
            if (!p) return std::nullopt;
            first = std::move(p->first);
            regex = p->second;
        } else {
            auto result = ""s;
            auto escaped = false;

            while (!regex.empty()) {
                if (escaped) {
                    result.push_back(regex[0]);
                    escaped = false;
                } else {
                    if (regex[0] == '\\') {
                        escaped = true;
                    } else if (regex[0] == '*') {
                        if (regex.size() > 1) {
                            regex = std::string_view(regex.data() - 1, regex.size() + 1);
                        }
                        break;
                    } else if (regex[0] == '|' || regex[0] == '(') {
                        break;
                    }
                }
                regex.remove_prefix(1);
            }

            if (regex.empty()) {
                return std::optional(std::pair(chef::_regex::regex {std::move(result)}, ""sv));
            }
            first.value = std::move(result);
        }
        if (regex.empty() || regex[0] == skip) {
            return std::optional(std::pair(std::move(first), regex));
        }

        if (regex[0] == '|') return parse_ll1_altp(std::move(first), regex);
        if (regex[0] == '*') {
            regex.remove_prefix(1);
            first = chef::_regex::regex {chef::_regex::kleene_star {
                std::make_unique<chef::_regex::regex>(chef::_regex::regex {std::move(first)})}};

            if (regex.empty() || regex[0] == skip) {
                return std::optional(std::pair(std::move(first), regex));
            }
        }

        return parse_ll1_seqp(std::move(first), regex);
    }

    auto _parse_regex(std::string_view regex) -> chef::_regex::regex
    {
        // regex   ::= <paren> | <seq> | <alt> | <kleene> | <literal>;
        // literal ::= /.*/;
        // paren   ::= '(' <regex> ')';
        // seq     ::= <regex> <regex>*;
        // alt     ::= <regex> | <regex> '|' <alt>;
        // kleene  ::= <regex> '*';

        // LL(1)-ify:
        // regex ::= <paren> | <literal> | <regex> <regex> <regex>* | (<regex> '|' <regex> | <regex>
        //          '|' <alt>) | (<regex> '*');
        //
        // regex  ::= <paren> <regex'> | <literal> <regex'>;
        // regex' ::= <regex> <regex>* | '|' <regex> | '|' <alt> | '*' | $;
        //
        // So:
        // regex ::= <simple> <regex'>;
        // simple ::= <paren> | <literal>;
        // regex' ::= <seq'> | <alt'> | '*' | $;
        // seq' ::= <regex> <regex>*;
        // alt' ::= '|' <regex> <alt'> | '|' <regex>;

        auto result = parse_ll1_re(regex);
        assert(result);
        assert(result->second.empty());
        return std::move(result->first);
    }
}
