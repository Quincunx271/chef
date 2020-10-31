#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace chef::_regex {
    struct regex;

    struct alternative {
        std::vector<std::unique_ptr<regex>> alternatives;
    };

    struct sequence {
        std::vector<std::unique_ptr<regex>> sequence;
    };

    struct kleene_star {
        std::unique_ptr<regex> item;
    };

    struct regex {
        using value_type = std::variant<std::string, alternative, sequence, kleene_star>;
        value_type value;
    };
}
