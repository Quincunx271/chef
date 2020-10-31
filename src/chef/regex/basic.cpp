#include "./basic.hpp"

#include <optional>

namespace chef {
    static auto basic_matches(chef::_regex::regex const& re, std::string_view str)
        -> std::optional<std::string_view>;

    static auto basic_matches(std::string_view literal, std::string_view str)
        -> std::optional<std::string_view>
    {
        if (str.size() < literal.size()) return std::nullopt;
        if (literal != str.substr(0, literal.size())) return std::nullopt;
        str.remove_prefix(literal.size());

        return std::optional(str);
    }

    static auto basic_matches(chef::_regex::alternative const& alt, std::string_view str)
        -> std::optional<std::string_view>
    {
        for (auto const& option : alt.alternatives) {
            // Incorrect; this doesn't backtrack if future options fail in a sequence...
            if (auto r = basic_matches(*option, str)) { return r; }
        }

        return std::nullopt;
    }

    static auto basic_matches(chef::_regex::sequence const& seq, std::string_view str)
        -> std::optional<std::string_view>
    {
        for (auto const& it : seq.sequence) {
            if (auto r = basic_matches(*it, str)) {
                str = *r;
            } else {
                return std::nullopt;
            }
        }

        return std::optional(str);
    }

    static auto basic_matches(chef::_regex::kleene_star const& star, std::string_view str)
        -> std::optional<std::string_view>
    {
        while (auto r = basic_matches(*star.item, str)) {
            str = *r;
        }

        return std::optional(str);
    }

    static auto basic_matches(chef::_regex::regex const& re, std::string_view str)
        -> std::optional<std::string_view>
    {
        return std::visit([str](auto const& re) { return basic_matches(re, str); }, re.value);
    }

    bool basic_regex_engine::matches(chef::_regex::regex const& re, std::string_view str)
    {
        auto match = basic_matches(re, str);
        return match.has_value() && match.value().empty();
    }
}
