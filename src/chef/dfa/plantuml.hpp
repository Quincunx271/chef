#pragma once

#include <iosfwd>

namespace chef {
    template <typename FA>
    struct to_plantuml {
        explicit to_plantuml(FA const& fa)
            : fa {&fa}
        { }

        friend auto operator<<(std::ostream& out, to_plantuml const& value) -> std::ostream&
        {
            return do_write(out, value);
        }

    private:
        FA const* fa;

        static auto do_write(std::ostream& out, to_plantuml const& value) -> std::ostream&;
    };
}
