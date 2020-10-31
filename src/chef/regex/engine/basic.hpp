#pragma once

#include <string_view>

#include <chef/regex/language.hpp>

namespace chef {
    struct basic_regex_engine {
        static bool matches(chef::_regex::regex const&, std::string_view);
    };
}
