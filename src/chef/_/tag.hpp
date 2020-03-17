#pragma once

namespace chef {
    template <typename T>
    struct _tag_t {};

    template <typename T>
    constexpr auto _tag = _tag_t<T>();
}
