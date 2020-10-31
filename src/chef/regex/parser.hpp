#pragma once

#include <string_view>

#include <chef/regex/language.hpp>

namespace chef {
    auto _parse_regex(std::string_view regex) -> chef::_regex::regex;

    auto parse_regex
        = [](std::string_view regex) -> chef::_regex::regex { return chef::_parse_regex(regex); };
}
